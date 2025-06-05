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
#include <QIcon>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QApplication>
#include <QStyle>
#include <QFontMetrics>

// Custom item delegate for image list
class ImageListItemDelegate : public QStyledItemDelegate {
public:
    explicit ImageListItemDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        painter->save();

        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        // Draw selection background
        if (opt.state & QStyle::State_Selected) {
            painter->fillRect(opt.rect, opt.palette.highlight());
        } else if (opt.state & QStyle::State_MouseOver) {
            QColor hoverColor = opt.palette.highlight().color();
            hoverColor.setAlpha(40);
            painter->fillRect(opt.rect, hoverColor);
        }

        // Draw icon
        int iconSize = opt.rect.height() - 8;
        QRect iconRect(opt.rect.x() + 4, opt.rect.y() + 4, iconSize, iconSize);
        QIcon icon = QIcon::fromTheme("distributor-logo");
        icon.paint(painter, iconRect, Qt::AlignCenter,
                   opt.state & QStyle::State_Selected ? QIcon::Selected : QIcon::Normal);

        // Draw text
        painter->setPen(opt.state & QStyle::State_Selected ?
        opt.palette.highlightedText().color() :
        opt.palette.text().color());
        QRect textRect = opt.rect.adjusted(iconSize + 12, 0, -4, 0);
        QString elidedText = opt.fontMetrics.elidedText(opt.text, Qt::ElideRight, textRect.width());
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, elidedText);

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size.setHeight(qMax(size.height(), 36)); // Minimum row height
        return size;
    }
};

CreateContainerDialog::CreateContainerDialog(Backend *backend, const QString &terminal, QWidget *parent)
: QDialog(parent), m_backend(backend), m_terminal(terminal), m_progressDialog(nullptr), m_createProcess(nullptr)
{
    // Window setup
    setWindowTitle("Create New Container");
    resize(700, 500);
    setWindowIcon(QIcon::fromTheme("computer"));

        // Setup form layout
        QFormLayout *formLayout = new QFormLayout;
        formLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
        formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        formLayout->setLabelAlignment(Qt::AlignRight);
        formLayout->setContentsMargins(8, 8, 8, 8);
        formLayout->setSpacing(12);

        m_nameEdit = new QLineEdit(this);
        m_nameEdit->setPlaceholderText("my-container");
        formLayout->addRow("Container Name:", m_nameEdit);

        m_searchEdit = new QLineEdit(this);
        m_searchEdit->setPlaceholderText("Search images...");
        connect(m_searchEdit, &QLineEdit::textChanged, this, &CreateContainerDialog::searchImages);
        formLayout->addRow("Search Images:", m_searchEdit);

        m_imageList = new QListWidget(this);
        m_imageList->setItemDelegate(new ImageListItemDelegate(this));
        m_imageList->setIconSize(QSize(32, 32));
        m_imageList->setSelectionMode(QAbstractItemView::SingleSelection);
        m_imageList->setAlternatingRowColors(true);
        formLayout->addRow("Available Images:", m_imageList);

        m_homeEdit = new QLineEdit(this);
        m_homeEdit->setPlaceholderText("Leave empty for default");
        formLayout->addRow("Custom Home Path:", m_homeEdit);

        m_volumesEdit = new QLineEdit(this);
        m_volumesEdit->setPlaceholderText("e.g., /host/path:/container/path (comma separate multiple)");
        formLayout->addRow("Additional Volumes:", m_volumesEdit);

        m_initCheckbox = new QCheckBox("Enable systemd init", this);
        m_initCheckbox->setToolTip("Enable systemd as init system inside the container");
        formLayout->addRow("Init System:", m_initCheckbox);

        // Setup buttons
        QHBoxLayout *buttonLayout = new QHBoxLayout;
        buttonLayout->setSpacing(10);
        buttonLayout->setContentsMargins(0, 10, 0, 0);

        QPushButton *refreshButton = new QPushButton("Refresh Images", this);
        refreshButton->setIcon(QIcon::fromTheme("view-refresh"));
        connect(refreshButton, &QPushButton::clicked, this, &CreateContainerDialog::refreshImages);
        buttonLayout->addWidget(refreshButton);

        buttonLayout->addStretch();

        QPushButton *cancelButton = new QPushButton("Cancel", this);
        cancelButton->setIcon(QIcon::fromTheme("dialog-cancel"));
        connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
        buttonLayout->addWidget(cancelButton);

        QPushButton *createButton = new QPushButton("Create", this);
        createButton->setIcon(QIcon::fromTheme("dialog-ok"));
        createButton->setDefault(true);
        connect(createButton, &QPushButton::clicked, this, &CreateContainerDialog::startContainerCreation);
        buttonLayout->addWidget(createButton);

        // Main layout
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(8, 8, 8, 8);
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
        QString distro = image["distro"];

        QListWidgetItem *item = new QListWidgetItem(url, m_imageList);
        item->setData(Qt::UserRole, url);
        item->setToolTip(url);

        // Set distro-specific icon if available
        QString iconName = QString("distributor-logo-%1").arg(distro);
        if (QIcon::hasThemeIcon(iconName)) {
            item->setIcon(QIcon::fromTheme(iconName));
        } else {
            item->setIcon(QIcon::fromTheme("distributor-logo"));
        }
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
        QString distro = image["distro"];

        QListWidgetItem *item = new QListWidgetItem(url, m_imageList);
        item->setData(Qt::UserRole, url);
        item->setToolTip(url);

        // Set distro-specific icon if available
        QString iconName = QString("distributor-logo-%1").arg(distro);
        if (QIcon::hasThemeIcon(iconName)) {
            item->setIcon(QIcon::fromTheme(iconName));
        } else {
            item->setIcon(QIcon::fromTheme("distributor-logo"));
        }
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
        m_backend->enterContainer(containerName(), m_terminal);
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
