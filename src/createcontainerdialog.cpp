#include "createcontainerdialog.h"
#include "backend.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

CreateContainerDialog::CreateContainerDialog(Backend *backend, QWidget *parent)
: QDialog(parent), m_backend(backend)
{
    setWindowTitle("Create New Container");
    resize(600, 400);

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
    formLayout->addRow("Additional Volumes (comma separated):", m_volumesEdit);

    m_initCheckbox = new QCheckBox("Enable systemd init", this);
    formLayout->addRow(m_initCheckbox);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *refreshButton = new QPushButton("Refresh Images", this);
    connect(refreshButton, &QPushButton::clicked, this, &CreateContainerDialog::refreshImages);
    buttonLayout->addWidget(refreshButton);

    QPushButton *createButton = new QPushButton("Create", this);
    connect(createButton, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(createButton);

    QPushButton *cancelButton = new QPushButton("Cancel", this);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

    refreshImages();
}

void CreateContainerDialog::refreshImages()
{
    m_imageList->clear();
    QList<QMap<QString, QString>> images = m_backend->getAvailableImages();

    for (const auto &image : images) {
        QString url = image["url"];
        QListWidgetItem *item = new QListWidgetItem(url, m_imageList);
        item->setData(Qt::UserRole, url);
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
        QListWidgetItem *item = new QListWidgetItem(url, m_imageList);
        item->setData(Qt::UserRole, url);
    }
}

QString CreateContainerDialog::containerName() const { return m_nameEdit->text(); }

QString CreateContainerDialog::imageUrl() const
{
    if (m_imageList->currentItem()) {
        return m_imageList->currentItem()->data(Qt::UserRole).toString();
    }
    return QString();
}

QString CreateContainerDialog::homePath() const { return m_homeEdit->text(); }

bool CreateContainerDialog::useInit() const { return m_initCheckbox->isChecked(); }

QStringList CreateContainerDialog::volumes() const
{
    return m_volumesEdit->text().split(',', Qt::SkipEmptyParts);
}