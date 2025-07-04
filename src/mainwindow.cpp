// SPDX-License-Identifier: GPL-2.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2025 Hadi Chokr <hadichokr@icloud.com>

#include "mainwindow.h"
#include "appsdialog.h"
#include "backend.h"
#include "createcontainerdialog.h"
#include "terminalutils.h"

// Custom delegate for container list items
class ContainerItemDelegate : public QStyledItemDelegate
{
public:
    explicit ContainerItemDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        painter->save();

        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        // Draw background (hover/selection)
        if (opt.state & QStyle::State_Selected) {
            painter->fillRect(opt.rect, opt.palette.highlight());
        } else if (opt.state & QStyle::State_MouseOver) {
            QColor hoverColor = opt.palette.highlight().color();
            hoverColor.setAlpha(40);
            painter->fillRect(opt.rect, hoverColor);
        }

        // Container data
        QString image = index.data(Qt::UserRole + 3).toString();
        QString distro = index.data(Qt::UserRole + 4).toString();
        QString iconPath = index.data(Qt::UserRole + 5).toString();
        bool isReady = index.data(Qt::UserRole + 6).toBool();
        bool isEmptyPlaceholder = index.data(Qt::UserRole + 7).toBool();

        // Icon rectangle
        int iconSize = opt.rect.height() - 8;
        QRect iconRect(opt.rect.x() + 4, opt.rect.y() + 4, iconSize, iconSize);

        // Load icon
        QPixmap icon;

        if (!iconPath.isEmpty()) {
            icon.load(iconPath);
        }

        if (icon.isNull()) {
            if (isEmptyPlaceholder) {
                icon = QIcon(":/icons/tux.svg").pixmap(iconSize, iconSize);
            } else if (isReady) {
                icon = QIcon::fromTheme("preferences-virtualization-container").pixmap(iconSize, iconSize);
            }
        }

        // Falls weiterhin kein Icon da ist, versuche es mit altIconPath
        if (icon.isNull()) {
            QVariant altIconData = index.data(Qt::UserRole + 5);
            if (altIconData.isValid() && !altIconData.isNull()) {
                QString altPath = altIconData.toString();
                QPixmap altIcon;
                altIcon.load(altPath);
                if (!altIcon.isNull()) {
                    icon = altIcon;
                }
            }
        }

        // Zeichne Icon
        if (!icon.isNull()) {
            icon = icon.scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            painter->drawPixmap(iconRect, icon);
        }

        // Haupttext (Name)
        QRect nameRect = opt.rect.adjusted(iconSize + 12, 0, -30, -opt.rect.height() / 2);
        QString elidedName = opt.fontMetrics.elidedText(opt.text, Qt::ElideRight, nameRect.width());
        painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, elidedName);

        // Sekundärtext (Image)
        QFont smallFont = opt.font;
        smallFont.setPointSize(smallFont.pointSize() - 2);
        painter->setFont(smallFont);

        QColor textColor = opt.palette.text().color();
        textColor.setAlpha(180);
        painter->setPen(textColor);

        QRect imageRect = opt.rect.adjusted(iconSize + 12, opt.rect.height() / 2, -30, 0);
        QString elidedImage = opt.fontMetrics.elidedText(image, Qt::ElideRight, imageRect.width());
        painter->drawText(imageRect, Qt::AlignLeft | Qt::AlignVCenter, elidedImage);

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size.setHeight(qMax(size.height(), 48));
        return size;
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Show loading UI immediately
    setupLoadingUI();

    // Load saved terminal preference
    QSettings settings;
    preferredTerminal = settings.value("terminal/preferred", "xterm").toString();

    backend = new Backend(this);
    connect(backend, &Backend::availableBackendsChanged, this, &MainWindow::onBackendsAvailable);
    connect(backend, &Backend::assembleFinished, this, &MainWindow::onAssembleFinished);
    connect(backend, &Backend::containersFetched, this, &MainWindow::handleContainersFetched);

    setWindowTitle(tr("Kontainer"));
    resize(850, 600);
    setWindowIcon(QIcon::fromTheme("preferences-virtualization-container"));
}

void MainWindow::refreshContainers()
{
    containerList->clear();

    // Add inline spinner
    QListWidgetItem *loadingItem = new QListWidgetItem(containerList);
    containerList->addItem(loadingItem);

    QWidget *loadingWidget = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(loadingWidget);
    layout->setContentsMargins(6, 6, 6, 6);

    QLabel *label = new QLabel(i18n("Fetching containers..."));
    QProgressBar *progress = new QProgressBar();
    progress->setRange(0, 0); // Indeterminate
    progress->setMaximumHeight(16);
    progress->setTextVisible(false);
    progress->setFixedWidth(100);

    layout->addWidget(progress);
    layout->addSpacing(10);
    layout->addWidget(label);
    layout->addStretch();

    loadingWidget->setLayout(layout);

    containerList->setItemWidget(loadingItem, loadingWidget);

    backend->fetchContainersAsync();
}

// New slot to handle fetched containers
void MainWindow::handleContainersFetched(const QList<QMap<QString, QString>> &containers)
{
    qDebug() << "handleContainersFetched(): received" << containers.size() << "containers";

    containerList->clear(); // removes spinner etc.

    if (containers.isEmpty()) {
        qDebug() << "No containers found. Adding placeholder with Tux icon.";
        QListWidgetItem *item = new QListWidgetItem(i18n("No containers found"), containerList);
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        item->setData(Qt::UserRole + 5, QString(":/icons/tux.svg")); // Set icon path for delegate
    } else {
        for (const auto &container : containers) {
            QListWidgetItem *item = new QListWidgetItem(container["name"], containerList);
            item->setData(Qt::UserRole + 3, container["image"]);
            item->setData(Qt::UserRole + 4, container["distro"]);
            item->setData(Qt::UserRole + 5, container["icon"]);
        }
    }

    currentContainer.clear();
    updateButtonStates();
}

void MainWindow::setupLoadingUI()
{
    QWidget *loadingWidget = new QWidget(this);
    QVBoxLayout *loadingLayout = new QVBoxLayout(loadingWidget);
    loadingLayout->setAlignment(Qt::AlignCenter);

    // Create a simple text-based loading indicator
    QLabel *loadingText = new QLabel(i18n("Checking for available container backends..."), loadingWidget);
    loadingText->setAlignment(Qt::AlignCenter);

    // Add a progress bar as a simple spinner replacement
    QProgressBar *progressBar = new QProgressBar(loadingWidget);
    progressBar->setRange(0, 0); // Indeterminate mode
    progressBar->setTextVisible(false);

    loadingLayout->addWidget(loadingText);
    loadingLayout->addWidget(progressBar);

    setCentralWidget(loadingWidget);
}

void MainWindow::onBackendsAvailable(const QStringList &backends)
{
    if (backends.isEmpty()) {
        QMessageBox::critical(this, i18n("Error"), i18n("No container backends found. Please install either distrobox or toolbox."));
        // Setup minimal UI even without backends
    }

    // Now setup the full UI
    setupUI();
    refreshContainers();
}

MainWindow::~MainWindow()
{
    // Save terminal preference
    QSettings settings;
    settings.setValue("terminal/preferred", preferredTerminal);
}

void MainWindow::setupUI()
{
    // Clear loading widget
    if (centralWidget()) {
        centralWidget()->deleteLater();
    }

    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(12);

    // Left panel - Container list
    containerList = new QListWidget(this);
    containerList->setItemDelegate(new ContainerItemDelegate(this));
    containerList->setIconSize(QSize(32, 32));
    containerList->setSelectionMode(QAbstractItemView::SingleSelection);
    containerList->setAlternatingRowColors(true);
    connect(containerList, &QListWidget::itemSelectionChanged, [this]() {
        if (containerList->currentItem()) {
            currentContainer = containerList->currentItem()->text();
            updateButtonStates();
        }
    });

    // Right panel - Action buttons
    QWidget *rightPanel = new QWidget(centralWidget);
    rightPanel->setFixedWidth(180);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(8);

    enterBtn = new QPushButton(QIcon::fromTheme("system-run"), i18n("Enter"), rightPanel);
    enterBtn->setToolTip(i18n("Enter the selected container"));
    connect(enterBtn, &QPushButton::clicked, this, &MainWindow::enterContainer);

    deleteBtn = new QPushButton(QIcon::fromTheme("edit-delete"), i18n("Delete"), rightPanel);
    deleteBtn->setToolTip(i18n("Delete the selected container"));
    connect(deleteBtn, &QPushButton::clicked, this, &MainWindow::deleteContainer);

    appsBtn = new QPushButton(QIcon::fromTheme("applications-other"), i18n("Applications"), rightPanel);
    appsBtn->setToolTip(i18n("Manage exported applications"));
    connect(appsBtn, &QPushButton::clicked, this, &MainWindow::showAppsDialog);

    upgradeBtn = new QPushButton(QIcon::fromTheme("system-software-update"), i18n("Upgrade"), rightPanel);
    upgradeBtn->setToolTip(i18n("Upgrade the selected container"));
    connect(upgradeBtn, &QPushButton::clicked, this, &MainWindow::upgradeContainer);

    QFrame *separator = new QFrame(rightPanel);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);

    QPushButton *refreshBtn = new QPushButton(QIcon::fromTheme("view-refresh"), i18n("Refresh"), rightPanel);
    refreshBtn->setToolTip(i18n("Refresh container list"));
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::refreshContainers);

    // Package installation buttons
    installDebBtn = new QPushButton(QIcon::fromTheme("application-x-deb"), i18n("Install .deb"), rightPanel);
    installDebBtn->setToolTip(i18n("Install Debian/Ubuntu package"));
    connect(installDebBtn, &QPushButton::clicked, this, &MainWindow::installDebPackage);

    installRpmBtn = new QPushButton(QIcon::fromTheme("application-x-rpm"), i18n("Install .rpm"), rightPanel);
    installRpmBtn->setToolTip(i18n("Install Fedora/OpenSUSE package"));
    connect(installRpmBtn, &QPushButton::clicked, this, &MainWindow::installRpmPackage);

    installArchBtn = new QPushButton(QIcon::fromTheme("application-x-tarz"), i18n("Install .pkg"), rightPanel);
    installArchBtn->setToolTip(i18n("Install Arch Linux package"));
    connect(installArchBtn, &QPushButton::clicked, this, &MainWindow::installArchPackage);

    rightLayout->addWidget(installDebBtn);
    rightLayout->addWidget(installRpmBtn);
    rightLayout->addWidget(installArchBtn);
    rightLayout->addWidget(separator);
    rightLayout->addWidget(enterBtn);
    rightLayout->addWidget(deleteBtn);
    rightLayout->addWidget(appsBtn);
    rightLayout->addWidget(upgradeBtn);
    rightLayout->addWidget(separator);
    rightLayout->addWidget(refreshBtn);
    rightLayout->addStretch();

    // Create toolbar
    QToolBar *toolBar = new QToolBar(this);
    toolBar->setMovable(false);
    toolBar->setIconSize(QSize(24, 24));

    // Left side buttons (create and upgrade all)
    addBtn = new QToolButton(toolBar);
    addBtn->setIcon(QIcon::fromTheme("list-add"));
    addBtn->setText(i18n("New Container"));
    addBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    addBtn->setToolTip(i18n("Create new container"));
    connect(addBtn, &QToolButton::clicked, this, &MainWindow::createNewContainer);
    toolBar->addWidget(addBtn);

    aBtn = new QToolButton(toolBar);
    aBtn->setIcon(QIcon::fromTheme("system-software-update"));
    aBtn->setText(i18n("Upgrade all Containers"));
    aBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    aBtn->setToolTip(i18n("Upgrades all containers"));
    connect(aBtn, &QToolButton::clicked, this, &MainWindow::upgradeAllContainers);
    toolBar->addWidget(aBtn);

    assembleBtn = new QToolButton(toolBar);
    assembleBtn->setIcon(QIcon::fromTheme("applications-development"));
    assembleBtn->setText(i18n("Assemble"));
    assembleBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    assembleBtn->setToolTip(i18n("Create container from INI file"));
    connect(assembleBtn, &QToolButton::clicked, this, &MainWindow::assembleContainer);
    toolBar->addWidget(assembleBtn);

    // Add expanding spacer between left and right sections
    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolBar->addWidget(spacer);

    // Right side - terminal selection
    QLabel *terminalLabel = new QLabel(i18n("Terminal:"), toolBar);
    toolBar->addWidget(terminalLabel);

    QComboBox *terminalSelector = new QComboBox(toolBar);
    auto terminalInfoMap = getTerminalInfoMap();

    // Get available terminals from backend
    QStringList availableTerminals = backend->getAvailableTerminals();
    hasTerminal = !availableTerminals.isEmpty();

    // Populate terminal selector
    for (const QString &term : availableTerminals) {
        TerminalInfo info = terminalInfoMap[term];
        QIcon icon =
            QIcon::fromTheme(info.icon, isFlatpakTerminal(term) ? QIcon::fromTheme("application-x-executable") : QIcon::fromTheme("utilities-terminal"));
        terminalSelector->addItem(icon, term);
    }

    // Add separator if we have both regular and Flatpak terminals
    bool hasRegular = std::any_of(availableTerminals.begin(), availableTerminals.end(), [this](const QString &term) {
        return !isFlatpakTerminal(term);
    });
    bool hasFlatpak = std::any_of(availableTerminals.begin(), availableTerminals.end(), [this](const QString &term) {
        return isFlatpakTerminal(term);
    });

    if (hasRegular && hasFlatpak) {
        for (int i = 0; i < terminalSelector->count(); ++i) {
            if (isFlatpakTerminal(terminalSelector->itemText(i))) {
                terminalSelector->insertSeparator(i);
                break;
            }
        }
    }

    if (availableTerminals.isEmpty()) {
        preferredTerminal.clear(); // kein Terminal vorhanden
        enterBtn->setEnabled(false);
        enterBtn->setToolTip(i18n("No terminal emulator found - output will be shown in dialog"));
    } else {
        int index = terminalSelector->findText(preferredTerminal);
        if (index >= 0) {
            terminalSelector->setCurrentIndex(index);
        } else {
            preferredTerminal = terminalSelector->itemText(0);
            QSettings settings;
            settings.setValue("terminal/preferred", preferredTerminal);
            terminalSelector->setCurrentIndex(0);
        }
        enterBtn->setEnabled(true);
    }

    connect(terminalSelector, &QComboBox::currentTextChanged, this, [this](const QString &term) {
        preferredTerminal = term;
        QSettings settings;
        settings.setValue("terminal/preferred", preferredTerminal);
    });

    toolBar->addWidget(terminalSelector);
    addToolBar(Qt::TopToolBarArea, toolBar);

    QComboBox *backendSelector = new QComboBox(toolBar);
    QStringList availableBackends = backend->availableBackends();
    QString currentBackend = backend->preferredBackend();

    for (const QString &backendName : availableBackends) {
        QIcon icon;

        if (backendName == "distrobox") {
            icon = QIcon(":/icons/distrobox.svg");
        } else if (backendName == "toolbox") {
            icon = QIcon(":/icons/toolbx.svg");
        } else {
            icon = QIcon::fromTheme("system-run"); // fallback
        }

        backendSelector->addItem(icon, backendName);
    }

    int backendIndex = backendSelector->findText(currentBackend);
    if (backendIndex >= 0) {
        backendSelector->setCurrentIndex(backendIndex);
    }

    connect(backendSelector, &QComboBox::currentTextChanged, this, [=](const QString &backendName) {
        backend->setPreferredBackend(backendName);
        upgradeBtn->setVisible(false);
        refreshContainers();
    });

    toolBar->addWidget(backendSelector);

    qDebug() << "Preferred Terminal:" << preferredTerminal;
    qDebug() << "Preferred Backend:" << backend->preferredBackend();

    mainLayout->addWidget(containerList, 1);
    mainLayout->addWidget(rightPanel);
    setCentralWidget(centralWidget);

    updateButtonStates();
}

void MainWindow::onAssembleFinished(const QString &result)
{
    if (progressDialog) {
        progressDialog->close();
        progressDialog->deleteLater();
        progressDialog = nullptr;
    }
    QMessageBox::information(this, i18n("Assemble Result"), result);
    refreshContainers();
}

void MainWindow::updateButtonStates()
{
    bool hasSelection = !currentContainer.isEmpty();
    if (!hasTerminal) {
        enterBtn->setVisible(false);
    } else {
        enterBtn->setEnabled(hasSelection);
    }

    deleteBtn->setEnabled(hasSelection);
    appsBtn->setEnabled(hasSelection);
    upgradeBtn->setEnabled(hasSelection);

    if (backend->preferredBackend() == "toolbox") {
        upgradeBtn->setVisible(false); // Upgrade selected container
    } else {
        upgradeBtn->setVisible(true);
    }

    if (hasSelection) {
        QString distro = getContainerDistro();
        installDebBtn->setVisible(distro == "deb");
        installRpmBtn->setVisible(distro == "rpm");
        installArchBtn->setVisible(distro == "arch");
    } else {
        installDebBtn->setVisible(false);
        installRpmBtn->setVisible(false);
        installArchBtn->setVisible(false);
    }
}

void MainWindow::deleteContainer()
{
    if (currentContainer.isEmpty())
        return;

    QMessageBox msgBox(this);
    msgBox.setWindowTitle(i18n("Confirm Deletion"));
    msgBox.setText(i18n("Delete container <b>%1</b>?", currentContainer));
    msgBox.setInformativeText(i18n("This action cannot be undone."));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setIcon(QMessageBox::Warning);

    if (msgBox.exec() == QMessageBox::Yes) {
        QString result = backend->deleteContainer(currentContainer);
        QMessageBox::information(this, i18n("Result"), result);
        refreshContainers();
    }
}

void MainWindow::showAppsDialog()
{
    if (currentContainer.isEmpty())
        return;

    // Set Busy Cursor
    QApplication::setOverrideCursor(Qt::BusyCursor);

    AppsDialog *dlg = new AppsDialog(backend, currentContainer, this);
    dlg->show();

    // Restore Cursor
    QApplication::restoreOverrideCursor();
}

void MainWindow::createNewContainer()
{
    CreateContainerDialog dialog(backend, preferredTerminal, this);
    if (dialog.exec() == QDialog::Accepted) {
        refreshContainers();
    }
}

QString MainWindow::getContainerDistro() const
{
    QString distro = backend->getContainerDistro(currentContainer);
    return distro;
}

void MainWindow::enterContainer()
{
    if (currentContainer.isEmpty())
        return;
    backend->enterContainer(currentContainer, preferredTerminal);
}

void MainWindow::assembleContainer()
{
    // Get the current backend from the Backend instance
    QString currentBackend = backend->preferredBackend();

    if (currentBackend == "toolbox") {
        QMessageBox::information(this,
                                 i18n("Function Not Supported"),
                                 i18n("Toolbox backend doesn't support container assembly.\n\n"
                                      "Please switch to Distrobox backend to use this feature."));
        return;
    }

    QString iniFile = QFileDialog::getOpenFileName(this, i18n("Select Distrobox INI File"), QDir::homePath(), i18n("INI Files (*.ini);;All Files (*)"));

    if (!iniFile.isEmpty()) {
        progressDialog = new QProgressDialog(i18n("Assembling container..."), i18n("Cancel"), 0, 0, this);
        progressDialog->setWindowModality(Qt::WindowModal);
        progressDialog->setCancelButton(nullptr);
        progressDialog->show();

        backend->assembleContainer(iniFile);
    }
}

void MainWindow::upgradeAllContainers()
{
    // Get the current backend from the Backend instance
    QString currentBackend = backend->preferredBackend();

    if (currentBackend == "toolbox") {
        QMessageBox::information(this,
                                 i18n("Function Not Supported"),
                                 i18n("Toolbox backend doesn't support mass container upgrades.\n\n"
                                      "Please switch to Distrobox backend to use this feature."));
        return;
    }

    qDebug() << "[upgradeAllContainers] Called with backend:" << currentBackend;

    if (preferredTerminal.isEmpty()) {
        qDebug() << "[upgradeAllContainers] No preferred terminal. Using internal upgrade.";
        setupProgressDialog(i18n("Upgrading all containers..."));

        connect(backend, &Backend::outputReceived, this, &MainWindow::appendCommandOutput);
        connect(backend, &Backend::upgradeAllFinished, this, [this](const QString &output) {
            appendCommandOutput(output);
            progressDialog->close();
            progressDialog->deleteLater();
            progressDialog = nullptr;
        });
        backend->upgradeAllContainersNoTerminal();
    } else {
        qDebug() << "[upgradeAllContainers] Using preferred terminal:" << preferredTerminal;
        connect(backend, &Backend::upgradeAllFinished, this, [this](const QString &output) {
            // Terminal will handle its own output
        });
        backend->upgradeAllContainers(preferredTerminal);
    }
}

// New helper function to create and show progress dialog with output
void MainWindow::setupProgressDialog(const QString &title)
{
    progressDialog = new QProgressDialog(this);
    progressDialog->setWindowTitle(title);
    progressDialog->setLabelText(i18n("Processing..."));
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setCancelButton(nullptr);

    progressOutput = new QTextEdit(progressDialog);
    progressOutput->setReadOnly(true);
    progressOutput->setWordWrapMode(QTextOption::NoWrap);

    QVBoxLayout *layout = new QVBoxLayout(progressDialog);
    layout->addWidget(progressOutput);
    progressDialog->setLayout(layout);
    progressDialog->resize(600, 400);
    progressDialog->show();
}

// New function to append output to the progress dialog
void MainWindow::appendCommandOutput(const QString &output)
{
    if (progressOutput) {
        progressOutput->append(output);
        // Auto-scroll to bottom
        QTextCursor cursor = progressOutput->textCursor();
        cursor.movePosition(QTextCursor::End);
        progressOutput->setTextCursor(cursor);
    }
}

void MainWindow::installDebPackage()
{
    qDebug() << "[installDebPackage] Called";

    if (currentContainer.isEmpty()) {
        qDebug() << "[installDebPackage] No container selected. Aborting.";
        return;
    }

    QString filePath = QFileDialog::getOpenFileName(this, i18n("Select .deb Package"), QDir::homePath(), i18n("Debian Packages (*.deb)"));
    qDebug() << "[installDebPackage] Selected file:" << filePath;

    if (!filePath.isEmpty()) {
        if (preferredTerminal.isEmpty()) {
            qDebug() << "[installDebPackage] No preferred terminal. Using internal install.";
            setupProgressDialog(i18n("Installing .deb package..."));

            connect(backend, &Backend::outputReceived, this, &MainWindow::appendCommandOutput);
            connect(backend, &Backend::debInstallFinished, this, [this](const QString &output) {
                appendCommandOutput(output);
                progressDialog->close();
                progressDialog->deleteLater();
                progressDialog = nullptr;
            });
            backend->installDebPackageNoTerminal(currentContainer, filePath);
        } else {
            qDebug() << "[installDebPackage] Using preferred terminal:" << preferredTerminal;
            connect(backend, &Backend::debInstallFinished, this, [](const QString &) {
                // Terminal will handle its own output
            });
            backend->installDebPackage(preferredTerminal, currentContainer, filePath);
        }
    } else {
        qDebug() << "[installDebPackage] File selection canceled.";
    }
}

void MainWindow::installRpmPackage()
{
    qDebug() << "[installRpmPackage] Called";

    if (currentContainer.isEmpty()) {
        qDebug() << "[installRpmPackage] No container selected. Aborting.";
        return;
    }

    QString filePath = QFileDialog::getOpenFileName(this, i18n("Select .rpm Package"), QDir::homePath(), i18n("RPM Packages (*.rpm)"));
    qDebug() << "[installRpmPackage] Selected file:" << filePath;

    if (!filePath.isEmpty()) {
        if (preferredTerminal.isEmpty()) {
            qDebug() << "[installRpmPackage] No preferred terminal. Using internal install.";
            setupProgressDialog(i18n("Installing .rpm package..."));

            connect(backend, &Backend::outputReceived, this, &MainWindow::appendCommandOutput);
            connect(backend, &Backend::rpmInstallFinished, this, [this](const QString &output) {
                appendCommandOutput(output);
                progressDialog->close();
                progressDialog->deleteLater();
                progressDialog = nullptr;
            });
            backend->installRpmPackageNoTerminal(currentContainer, filePath);
        } else {
            qDebug() << "[installRpmPackage] Using preferred terminal:" << preferredTerminal;
            connect(backend, &Backend::rpmInstallFinished, this, [](const QString &) {
                // Terminal will handle its own output
            });
            backend->installRpmPackage(preferredTerminal, currentContainer, filePath);
        }
    } else {
        qDebug() << "[installRpmPackage] File selection canceled.";
    }
}

void MainWindow::installArchPackage()
{
    qDebug() << "[installArchPackage] Called";

    if (currentContainer.isEmpty()) {
        qDebug() << "[installArchPackage] No container selected. Aborting.";
        return;
    }

    QString filePath = QFileDialog::getOpenFileName(this, i18n("Select Arch Package"), QDir::homePath(), i18n("Arch Packages (*.pkg.tar.*)"));
    qDebug() << "[installArchPackage] Selected file:" << filePath;

    if (!filePath.isEmpty()) {
        if (preferredTerminal.isEmpty()) {
            qDebug() << "[installArchPackage] No preferred terminal. Using internal install.";
            setupProgressDialog(i18n("Installing Arch package..."));

            connect(backend, &Backend::outputReceived, this, &MainWindow::appendCommandOutput);
            connect(backend, &Backend::archInstallFinished, this, [this](const QString &output) {
                appendCommandOutput(output);
                progressDialog->close();
                progressDialog->deleteLater();
                progressDialog = nullptr;
            });
            backend->installArchPackageNoTerminal(currentContainer, filePath);
        } else {
            qDebug() << "[installArchPackage] Using preferred terminal:" << preferredTerminal;
            connect(backend, &Backend::archInstallFinished, this, [](const QString &) {
                // Terminal will handle its own output
            });
            backend->installArchPackage(preferredTerminal, currentContainer, filePath);
        }
    } else {
        qDebug() << "[installArchPackage] File selection canceled.";
    }
}

void MainWindow::upgradeContainer()
{
    qDebug() << "[upgradeContainer] Called";

    if (currentContainer.isEmpty()) {
        qDebug() << "[upgradeContainer] No container selected. Aborting.";
        return;
    }

    if (preferredTerminal.isEmpty()) {
        qDebug() << "[upgradeContainer] No preferred terminal. Using internal upgrade.";
        setupProgressDialog(i18n("Upgrading container..."));

        connect(backend, &Backend::outputReceived, this, &MainWindow::appendCommandOutput);
        connect(backend, &Backend::upgradeFinished, this, [this](const QString &output) {
            appendCommandOutput(output);
            progressDialog->close();
            progressDialog->deleteLater();
            progressDialog = nullptr;
        });
        backend->upgradeContainerNoTerminal(currentContainer);
    } else {
        qDebug() << "[upgradeContainer] Using preferred terminal:" << preferredTerminal;
        connect(backend, &Backend::upgradeFinished, this, [](const QString &) {
            // Terminal will handle its own output
        });
        backend->upgradeContainer(currentContainer, preferredTerminal);
    }
}
