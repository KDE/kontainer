// SPDX-License-Identifier: GPL-2.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2025 Hadi Chokr <hadichokr@icloud.com>

#include "createcontainerdialog.h"
#include "backend.h"

// Custom item delegate for image list
class ImageListItemDelegate : public QStyledItemDelegate
{
public:
    explicit ImageListItemDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
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

        // Get icon path and distro from item data
        QString iconPath = index.data(Qt::UserRole + 2).toString();
        QString distro = index.data(Qt::UserRole + 1).toString();

        // Icon drawing
        int iconSize = opt.rect.height() - 8;
        QRect iconRect(opt.rect.x() + 4, opt.rect.y() + 4, iconSize, iconSize);

        QPixmap icon;
        if (!iconPath.isEmpty()) {
            icon.load(iconPath); // Load from given path
        }

        if (icon.isNull()) {
            icon = QIcon(":/icons/tux.svg").pixmap(iconSize, iconSize);
            if (icon.isNull()) {
                // Final fallback, in case embedded icon also fails
                icon = QIcon::fromTheme("preferences-virtualization-container").pixmap(iconSize, iconSize);
            }
        }

        icon = icon.scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter->drawPixmap(iconRect, icon);

        // Draw text
        painter->setPen(opt.state & QStyle::State_Selected ? opt.palette.highlightedText().color() : opt.palette.text().color());
        QRect textRect = opt.rect.adjusted(iconSize + 12, 0, -4, 0);
        QString elidedText = opt.fontMetrics.elidedText(opt.text, Qt::ElideRight, textRect.width());
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, elidedText);

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size.setHeight(qMax(size.height(), 36)); // Minimum row height
        return size;
    }
};

CreateContainerDialog::CreateContainerDialog(Backend *backend, QWidget *parent)
    : QDialog(parent)
    , m_backend(backend)
    , m_progressDialog(nullptr)
    , m_createProcess(nullptr)
{
    // Window setup
    setWindowTitle(i18n("Create New Container"));
    resize(700, 500);
    setWindowIcon(QIcon::fromTheme("computer"));

    // Setup form layout
    QFormLayout *formLayout = new QFormLayout;
    formLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    formLayout->setLabelAlignment(Qt::AlignRight);
    formLayout->setContentsMargins(8, 8, 8, 8);
    formLayout->setSpacing(12);

    // Container name input
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText(i18n("my-container"));
    formLayout->addRow(i18n("Container name:"), m_nameEdit);

    // Image search and list layout
    QVBoxLayout *imageSearchLayout = new QVBoxLayout();
    imageSearchLayout->setSpacing(5); // Reduced spacing between search and list

    // Search bar layout
    QHBoxLayout *searchLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(i18n("Search images..."));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setStyleSheet("QLineEdit { padding: 3px; }");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &CreateContainerDialog::searchImages);

    // Add search edit to search layout
    searchLayout->addWidget(m_searchEdit);
    imageSearchLayout->addLayout(searchLayout);

    // Image list widget
    m_imageList = new QListWidget(this);
    m_imageList->setItemDelegate(new ImageListItemDelegate(this));
    m_imageList->setIconSize(QSize(32, 32));
    m_imageList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_imageList->setAlternatingRowColors(true);
    m_imageList->setFrameShape(QFrame::StyledPanel);
    m_imageList->setStyleSheet("QListWidget { border-top: none; }"); // Visually connected with search
    imageSearchLayout->addWidget(m_imageList);

    // Container widget for image search section
    QWidget *imageSearchWidget = new QWidget(this);
    imageSearchWidget->setLayout(imageSearchLayout);
    formLayout->addRow(i18n("Available images:"), imageSearchWidget);

    // Additional fields only for non-toolbox backends (e.g., Distrobox)
    if (backend->preferredBackend() != "toolbox") {
        m_homeEdit = new QLineEdit(this);
        m_homeEdit->setPlaceholderText(i18n("Leave empty for default"));
        formLayout->addRow(i18n("Custom home path:"), m_homeEdit);

        m_volumesEdit = new QLineEdit(this);
        m_volumesEdit->setPlaceholderText(i18n("e.g., /host/path:/container/path (comma separate multiple)"));
        formLayout->addRow(i18n("Additional volumes:"), m_volumesEdit);

        m_initCheckbox = new QCheckBox(i18n("Enable systemd init"), this);
        m_initCheckbox->setToolTip(i18n("Enable systemd as init system inside the container"));
        formLayout->addRow(i18n("Init system:"), m_initCheckbox);
    } else {
        m_homeEdit = nullptr;
        m_volumesEdit = nullptr;
        m_initCheckbox = nullptr;
    }

    // Buttons layout
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(10);
    buttonLayout->setContentsMargins(0, 10, 0, 0);

    // Refresh images button (hidden for toolbox backend)
    if (backend->preferredBackend() != "toolbox") {
        QPushButton *refreshButton = new QPushButton(i18n("Refresh Images"), this);
        refreshButton->setIcon(QIcon::fromTheme("view-refresh"));
        connect(refreshButton, &QPushButton::clicked, this, &CreateContainerDialog::refreshImages);
        buttonLayout->addWidget(refreshButton);
    }

    buttonLayout->addStretch();

    // Cancel button
    QPushButton *cancelButton = new QPushButton(i18n("Cancel"), this);
    cancelButton->setIcon(QIcon::fromTheme("dialog-cancel"));
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelButton);

    // Create button
    QPushButton *createButton = new QPushButton(i18n("Create"), this);
    createButton->setIcon(QIcon::fromTheme("dialog-ok"));
    createButton->setDefault(true);
    connect(createButton, &QPushButton::clicked, this, &CreateContainerDialog::startContainerCreation);
    buttonLayout->addWidget(createButton);

    // Main vertical layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

    // Initial images load
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
        QString displayText = image.value("display", image["url"]);
        QString distro = image["distro"];
        QString iconPath = image["icon"];

        QListWidgetItem *item = new QListWidgetItem(displayText, m_imageList);
        item->setData(Qt::UserRole, image["url"]); // Store URL in UserRole
        item->setData(Qt::UserRole + 1, distro); // Store distro in UserRole + 1
        item->setData(Qt::UserRole + 2, iconPath); // Store icon path in UserRole + 2
        item->setToolTip(image["url"]);
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
        QString iconPath = image["icon"];

        QListWidgetItem *item = new QListWidgetItem(url, m_imageList);
        item->setData(Qt::UserRole, url); // Store URL in UserRole
        item->setData(Qt::UserRole + 1, distro); // Store distro in UserRole + 1
        item->setData(Qt::UserRole + 2, iconPath); // Store icon path in UserRole + 2
        item->setToolTip(url);
    }
}

void CreateContainerDialog::startContainerCreation()
{
    QString name = containerName();
    QString image = imageUrl();
    QString home = m_homeEdit ? m_homeEdit->text().trimmed() : QString();
    bool init = m_initCheckbox ? m_initCheckbox->isChecked() : false;
    QStringList volumes = m_volumesEdit ? m_volumesEdit->text().split(',', Qt::SkipEmptyParts) : QStringList();

    // Validate inputs
    if (name.isEmpty()) {
        QMessageBox::warning(this, i18n("Error"), i18n("Please enter a container name"));
        return;
    }

    if (image.isEmpty()) {
        QMessageBox::warning(this, i18n("Error"), i18n("Please select an image"));
        return;
    }

    // Setup progress dialog
    if (m_progressDialog) {
        m_progressDialog->deleteLater();
    }
    m_progressDialog = new QProgressDialog(this);
    m_progressDialog->setWindowTitle(i18n("Creating Container"));
    m_progressDialog->setLabelText(i18n("Starting container creation..."));
    m_progressDialog->setRange(0, 0);
    m_progressDialog->setCancelButton(nullptr);
    m_progressDialog->show();

    // Disconnect any existing connections
    disconnect(m_backend, &Backend::containerOutput, this, nullptr);
    disconnect(m_backend, &Backend::containerCreationFinished, this, nullptr);

    // Connect signals
    connect(m_backend, &Backend::containerOutput, this, [this](const QString &output) {
        if (m_progressDialog) {
            m_progressDialog->setLabelText(output);
        }
    });

    connect(m_backend, &Backend::containerCreationFinished, this, [this, name](bool success, const QString &message) {
        if (m_progressDialog) {
            m_progressDialog->cancel();
            m_progressDialog->deleteLater();
            m_progressDialog = nullptr;
        }

        if (success) {
            QString successMsg = i18n(
                "Container '%1' created successfully!\n\n"
                "A terminal session with the new container will now open.",
                name);
            QMessageBox::information(this, i18n("Success"), successMsg);

            if (m_backend) {
                m_backend->enterContainer(name);
            }
            accept();
        } else {
            QMessageBox::critical(this, i18n("Error"), message);
        }
    });

    // Start container creation
    m_backend->createContainer(name, image, home, init, volumes);
}

void CreateContainerDialog::handleReadyRead()
{
    QString output = QString::fromUtf8(m_createProcess->readAll());
    if (!output.isEmpty()) {
        m_progressDialog->setLabelText(output);
        qDebug() << i18n("Process output:") << output;
    }
}

void CreateContainerDialog::handleErrorOccurred(QProcess::ProcessError error)
{
    m_progressDialog->cancel();
    QString errorMsg = i18n("Process error:") + QLatin1String(" ");
    switch (error) {
    case QProcess::FailedToStart:
        errorMsg += i18n("Failed to start");
        break;
    case QProcess::Crashed:
        errorMsg += i18n("Crashed");
        break;
    case QProcess::Timedout:
        errorMsg += i18n("Timed out");
        break;
    case QProcess::WriteError:
        errorMsg += i18n("Write error");
        break;
    case QProcess::ReadError:
        errorMsg += i18n("Read error");
        break;
    default:
        errorMsg += i18n("Unknown error");
        break;
    }
    errorMsg += "\n" + m_createProcess->errorString();

    QMessageBox::critical(this, i18n("Error"), errorMsg);
    m_createProcess->deleteLater();
    m_createProcess = nullptr;
}

void CreateContainerDialog::handleCreateFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_progressDialog->cancel();

    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        QString successMsg = i18n("Container “%1” created successfully!", containerName());
        m_progressDialog->setLabelText(successMsg);
        QMessageBox::information(this, i18n("Success"), successMsg);
        m_backend->enterContainer(containerName());
        accept();
    } else {
        QString errorOutput = QString::fromUtf8(m_createProcess->readAll());
        QString errorMsg = i18n("Container creation failed (exit code: %1)", exitCode);
        if (!errorOutput.isEmpty()) {
            errorMsg = i18n("Container creation failed (exit code: %1)\n\nError output: %2\n", exitCode, errorOutput);
        }
        QMessageBox::critical(this, i18n("Error"), errorMsg);
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
    return m_homeEdit ? m_homeEdit->text().trimmed() : QString();
}

QStringList CreateContainerDialog::volumes() const
{
    return m_volumesEdit ? m_volumesEdit->text().split(',', Qt::SkipEmptyParts) : QStringList();
}

bool CreateContainerDialog::useInit() const
{
    return m_initCheckbox ? m_initCheckbox->isChecked() : false;
}
