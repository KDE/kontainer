#include "createcontainerdialog.h"
#include "backend.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QDebug>
#include <QLineEdit>
#include <QCheckBox>

CreateContainerDialog::CreateContainerDialog(Backend *backend, QWidget *parent)
: QDialog(parent), m_backend(backend), m_progressDialog(nullptr), m_createProcess(nullptr)
{
    setWindowTitle("Create New Container");
    resize(600, 400);

    // Setup form layout
    QFormLayout *formLayout = new QFormLayout;

    m_nameEdit = new QLineEdit(this);
    formLayout->addRow("Container Name:", m_nameEdit);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Search images...");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &CreateContainerDialog::searchImages);
    formLayout->addRow("Search:", m_searchEdit);

    m_imageList = new QListWidget(this);
    m_imageList->setSelectionMode(QAbstractItemView::SingleSelection);
    formLayout->addRow("Available Images:", m_imageList);

    m_homeEdit = new QLineEdit(this);
    formLayout->addRow("Custom Home Path:", m_homeEdit);

    m_volumesEdit = new QLineEdit(this);
    m_volumesEdit->setPlaceholderText("e.g., /host/path:/container/path");
    formLayout->addRow("Additional Volumes:", m_volumesEdit);

    m_initCheckbox = new QCheckBox("Enable systemd init", this);
    formLayout->addRow(m_initCheckbox);

    // Setup buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *refreshButton = new QPushButton("Refresh Images", this);
    connect(refreshButton, &QPushButton::clicked, this, &CreateContainerDialog::refreshImages);
    buttonLayout->addWidget(refreshButton);

    QPushButton *createButton = new QPushButton("Create", this);
    createButton->setDefault(true);
    connect(createButton, &QPushButton::clicked, this, &CreateContainerDialog::startContainerCreation);
    buttonLayout->addWidget(createButton);

    QPushButton *cancelButton = new QPushButton("Cancel", this);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelButton);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

    refreshImages();
}

CreateContainerDialog::~CreateContainerDialog()
{
    if (m_createProcess) {
        if (m_createProcess->state() == QProcess::Running) {
            m_createProcess->terminate();
            m_createProcess->waitForFinished(1000);
        }
        delete m_createProcess;
    }
    delete m_progressDialog;
}

void CreateContainerDialog::refreshImages()
{
    m_imageList->clear();
    QList<QMap<QString, QString>> images = m_backend->getAvailableImages();

    for (const auto &image : images) {
        QString url = image["url"];
        QString name = image["name"];
        QListWidgetItem *item = new QListWidgetItem(name.isEmpty() ? url : name, m_imageList);
        item->setData(Qt::UserRole, url);
        item->setToolTip(url);
    }
}

void CreateContainerDialog::searchImages(const QString &query)
{
    if (query.isEmpty()) {
        refreshImages();
        return;
    }

    m_imageList->clear();
    QList<QMap<QString, QString>> images = m_backend->searchImages(query);

    for (const auto &image : images) {
        QString url = image["url"];
        QString name = image["name"];
        QListWidgetItem *item = new QListWidgetItem(name.isEmpty() ? url : name, m_imageList);
        item->setData(Qt::UserRole, url);
        item->setToolTip(url);
    }
}

void CreateContainerDialog::startContainerCreation()
{
    QString name = containerName();
    QString image = imageUrl();
    QString home = homePath();
    bool init = useInit();
    QStringList volumes = this->volumes();

    // Validate inputs
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a container name");
        return;
    }

    if (image.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select an image");
        return;
    }

    // Setup progress dialog
    m_progressDialog = new QProgressDialog(this);
    m_progressDialog->setWindowTitle("Creating Container");
    m_progressDialog->setLabelText("Starting container creation...");
    m_progressDialog->setRange(0, 0); // Indeterminate progress
    m_progressDialog->setCancelButton(nullptr);
    m_progressDialog->setModal(true);
    m_progressDialog->show();

    // Setup process
    m_createProcess = new QProcess(this);
    m_createProcess->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_createProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &CreateContainerDialog::handleCreateFinished);
    connect(m_createProcess, &QProcess::readyRead,
            this, &CreateContainerDialog::handleReadyRead);
    connect(m_createProcess, &QProcess::errorOccurred,
            this, &CreateContainerDialog::handleErrorOccurred);

    // Build command
    QStringList args = {"create", "-n", name, "-i", image, "-Y"};
    if (init) {
        args << "--init" << "--additional-packages" << "systemd";
    }
    if (!home.isEmpty()) {
        args << "--home" << home;
    }
    for (const QString &volume : volumes) {
        if (!volume.trimmed().isEmpty()) {
            args << "--volume" << volume.trimmed();
        }
    }

    qDebug() << "Executing:" << "distrobox" << args;
    m_createProcess->start("distrobox", args);
}

void CreateContainerDialog::handleReadyRead()
{
    QString output = QString::fromUtf8(m_createProcess->readAll());
    if (!output.isEmpty()) {
        m_progressDialog->setLabelText(output);
        qDebug() << "Process output:" << output;
    }
}

void CreateContainerDialog::handleErrorOccurred(QProcess::ProcessError error)
{
    m_progressDialog->cancel();
    QString errorMsg = "Process error: ";
    switch (error) {
        case QProcess::FailedToStart: errorMsg += "Failed to start"; break;
        case QProcess::Crashed: errorMsg += "Crashed"; break;
        case QProcess::Timedout: errorMsg += "Timed out"; break;
        case QProcess::WriteError: errorMsg += "Write error"; break;
        case QProcess::ReadError: errorMsg += "Read error"; break;
        default: errorMsg += "Unknown error"; break;
    }
    errorMsg += "\n" + m_createProcess->errorString();

    QMessageBox::critical(this, "Error", errorMsg);
    m_createProcess->deleteLater();
    m_createProcess = nullptr;
}

void CreateContainerDialog::handleCreateFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_progressDialog->cancel();

    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        QString successMsg = QString("Container '%1' created successfully!").arg(containerName());
        m_progressDialog->setLabelText(successMsg);
        QMessageBox::information(this, "Success", successMsg);
        m_backend->enterContainer(containerName());
        accept();
    } else {
        QString errorOutput = QString::fromUtf8(m_createProcess->readAll());
        QString errorMsg = QString("Container creation failed (exit code: %1)\n").arg(exitCode);
        if (!errorOutput.isEmpty()) {
            errorMsg += "\nError output:\n" + errorOutput;
        }
        QMessageBox::critical(this, "Error", errorMsg);
    }

    m_createProcess->deleteLater();
    m_createProcess = nullptr;
}

QString CreateContainerDialog::containerName() const
{
    return m_nameEdit->text().trimmed();
}

QString CreateContainerDialog::imageUrl() const
{
    if (m_imageList->currentItem()) {
        return m_imageList->currentItem()->data(Qt::UserRole).toString();
    }
    return QString();
}

QString CreateContainerDialog::homePath() const
{
    return m_homeEdit->text().trimmed();
}

bool CreateContainerDialog::useInit() const
{
    return m_initCheckbox->isChecked();
}

QStringList CreateContainerDialog::volumes() const
{
    return m_volumesEdit->text().split(',', Qt::SkipEmptyParts);
}
