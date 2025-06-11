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

        // Draw background
        if (opt.state & QStyle::State_Selected) {
            painter->fillRect(opt.rect, opt.palette.highlight());
        } else if (opt.state & QStyle::State_MouseOver) {
            QColor hoverColor = opt.palette.highlight().color();
            hoverColor.setAlpha(40);
            painter->fillRect(opt.rect, hoverColor);
        }

        // Get container data
        QString image = index.data(Qt::UserRole + 3).toString();
        QString distro = index.data(Qt::UserRole + 4).toString();

        // Draw icon with fallback logic
        int iconSize = opt.rect.height() - 8;
        QRect iconRect(opt.rect.x() + 4, opt.rect.y() + 4, iconSize, iconSize);

        QIcon icon = findBestIcon(distro);
        icon.paint(painter, iconRect, Qt::AlignCenter, QIcon::Normal);

        // Draw container name with accent color
        painter->setPen(opt.state & QStyle::State_Selected ? opt.palette.highlightedText().color() : opt.palette.text().color());

        QRect nameRect = opt.rect.adjusted(iconSize + 12, 0, -30, -opt.rect.height() / 2);
        QString elidedName = opt.fontMetrics.elidedText(opt.text, Qt::ElideRight, nameRect.width());
        painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, elidedName);

        // Draw image name below
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

private:
    QIcon findBestIcon(const QString &distro) const
    {
        if (distro.isEmpty() || distro == "unknown") {
            qDebug() << "No distro specified, using fallback icon";
            return getFallbackIcon();
        }

        // First try the exact symbolic icon path we know exists
        QString symbolicPath = QString("/usr/share/icons/char-white/apps/symbolic/distributor-logo-%1-symbolic.svg").arg(distro);
        if (QFile::exists(symbolicPath)) {
            qDebug() << "Found exact symbolic icon:" << symbolicPath;
            return QIcon(symbolicPath);
        }

        // Try alternative naming variations for symbolic icons
        QStringList symbolicVariations = {
            QString("/usr/share/icons/char-white/apps/symbolic/%1-symbolic.svg").arg(distro),
            QString("/usr/share/icons/char-white/apps/symbolic/%1-logo-symbolic.svg").arg(distro),
            QString("/usr/share/icons/char-white/apps/symbolic/%1linux-symbolic.svg").arg(distro),
            QString("/usr/share/icons/char-white/apps/symbolic/linux-%1-symbolic.svg").arg(distro)
        };

        for (const QString &path : symbolicVariations) {
            if (QFile::exists(path)) {
                qDebug() << "Found symbolic icon variation:" << path;
                return QIcon(path);
            }
        }

        // List of possible theme icon names to try (with and without -symbolic suffix)
        QStringList iconNames = {
            "distributor-logo-" + distro + "-symbolic",
            "distributor-logo-" + distro,
            distro + "-logo-symbolic",
            distro + "-logo",
            distro + "-symbolic",
            distro,
            "linux-" + distro + "-symbolic",
            "linux-" + distro,
            distro + "linux-symbolic",
            distro + "linux"
        };

        // First try theme icons
        for (const QString &iconName : iconNames) {
            QIcon icon = QIcon::fromTheme(iconName);
            if (!icon.isNull()) {
                qDebug() << "Found theme icon:" << iconName;
                return icon;
            }
            qDebug() << "Theme icon not found:" << iconName;
        }

        qDebug() << "No distro-specific icon found for" << distro << "- using fallback";
        return getFallbackIcon();
    }

    QIcon getFallbackIcon() const
    {
        // Try several fallback options
        QStringList fallbacks = {
            "preferences-virtualization-container",
            "virtualbox",
            "docker",
            "application-x-executable",
            "system-run"
        };

        for (const QString &fallback : fallbacks) {
            QIcon icon = QIcon::fromTheme(fallback);
            if (!icon.isNull()) {
                qDebug() << "Using fallback icon:" << fallback;
                return icon;
            }
        }

        qDebug() << "No fallback icons found - using default";
        return QIcon::fromTheme("application-x-executable");
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , backend(new Backend(this))
    , preferredTerminal("xterm")
{
    // Load saved terminal preference
    QSettings settings;
    preferredTerminal = settings.value("terminal/preferred", "xterm").toString();
    connect(backend, &Backend::assembleFinished, this, &MainWindow::onAssembleFinished);

    setWindowTitle("Kontainer");
    resize(600, 600);
    setWindowIcon(QIcon::fromTheme("preferences-virtualization-container"));

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

    enterBtn = new QPushButton(QIcon::fromTheme("system-run"), "Enter", rightPanel);
    enterBtn->setToolTip("Enter the selected container");
    connect(enterBtn, &QPushButton::clicked, this, &MainWindow::enterContainer);

    deleteBtn = new QPushButton(QIcon::fromTheme("edit-delete"), "Delete", rightPanel);
    deleteBtn->setToolTip("Delete the selected container");
    connect(deleteBtn, &QPushButton::clicked, this, &MainWindow::deleteContainer);

    appsBtn = new QPushButton(QIcon::fromTheme("applications-other"), "Applications", rightPanel);
    appsBtn->setToolTip("Manage exported applications");
    connect(appsBtn, &QPushButton::clicked, this, &MainWindow::showAppsDialog);

    upgradeBtn = new QPushButton(QIcon::fromTheme("system-software-update"), "Upgrade", rightPanel);
    upgradeBtn->setToolTip("Upgrade the selected container");
    connect(upgradeBtn, &QPushButton::clicked, this, &MainWindow::upgradeContainer);

    QFrame *separator = new QFrame(rightPanel);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);

    QPushButton *refreshBtn = new QPushButton(QIcon::fromTheme("view-refresh"), "Refresh", rightPanel);
    refreshBtn->setToolTip("Refresh container list");
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::refreshContainers);

    // Package installation buttons
    installDebBtn = new QPushButton(QIcon::fromTheme("application-x-deb"), "Install .deb", rightPanel);
    installDebBtn->setToolTip("Install Debian/Ubuntu package");
    connect(installDebBtn, &QPushButton::clicked, this, &MainWindow::installDebPackage);

    installRpmBtn = new QPushButton(QIcon::fromTheme("application-x-rpm"), "Install .rpm", rightPanel);
    installRpmBtn->setToolTip("Install Fedora/OpenSUSE package");
    connect(installRpmBtn, &QPushButton::clicked, this, &MainWindow::installRpmPackage);

    installArchBtn = new QPushButton(QIcon::fromTheme("application-x-tarz"), "Install .pkg", rightPanel);
    installArchBtn->setToolTip("Install Arch Linux package");
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
    addBtn->setText("New Container");
    addBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    addBtn->setToolTip("Create new container");
    connect(addBtn, &QToolButton::clicked, this, &MainWindow::createNewContainer);
    toolBar->addWidget(addBtn);

    aBtn = new QToolButton(toolBar);
    aBtn->setIcon(QIcon::fromTheme("system-software-update"));
    aBtn->setText("Upgrade all Containers");
    aBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    aBtn->setToolTip("Upgrades all containers");
    connect(aBtn, &QToolButton::clicked, this, &MainWindow::upgradeAllContainers);
    toolBar->addWidget(aBtn);

    assembleBtn = new QToolButton(toolBar);
    assembleBtn->setIcon(QIcon::fromTheme("applications-development"));
    assembleBtn->setText("Assemble");
    assembleBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    assembleBtn->setToolTip("Create container from INI file");
    connect(assembleBtn, &QToolButton::clicked, this, &MainWindow::assembleContainer);
    toolBar->addWidget(assembleBtn);

    // Add expanding spacer between left and right sections
    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolBar->addWidget(spacer);

    // Right side - terminal selection
    QLabel *terminalLabel = new QLabel("Terminal:", toolBar);
    toolBar->addWidget(terminalLabel);

    QComboBox *terminalSelector = new QComboBox(toolBar);
    auto terminalInfoMap = getTerminalInfoMap();

    // Check for regular terminals
    for (const auto &[term, info] : terminalInfoMap.asKeyValueRange()) {
        if (!isFlatpakTerminal(term)) {
            if (!QStandardPaths::findExecutable(term).isEmpty()) {
                QIcon icon = QIcon::fromTheme(info.icon);
                terminalSelector->addItem(icon, term);
            }
        }
    }

    // Check for Flatpak terminals
    for (const auto &[term, info] : terminalInfoMap.asKeyValueRange()) {
        if (isFlatpakTerminal(term)) {
            QProcess process;
            process.start("flatpak", {"list", "--app", "--columns=application"});
            if (process.waitForFinished() && process.readAllStandardOutput().contains(term.toUtf8())) {
                QIcon icon = QIcon::fromTheme(info.icon);
                terminalSelector->addItem(icon, term);
            }
        }
    }

    // Add separator if we have both types
    if (terminalSelector->count() > 0
        && (terminalSelector->findText("org.") != -1 || terminalSelector->findText("com.") != -1 || terminalSelector->findText("app.") != -1)) {
        terminalSelector->insertSeparator(terminalSelector->count());
    }

    // Set current terminal to saved preference or first available
    int index = terminalSelector->findText(preferredTerminal);
    if (index >= 0) {
        terminalSelector->setCurrentIndex(index);
    } else if (terminalSelector->count() > 0) {
        preferredTerminal = terminalSelector->itemText(0);
    }

    connect(terminalSelector, &QComboBox::currentTextChanged, this, [this](const QString &term) {
        preferredTerminal = term;
        // Save preference immediately
        QSettings settings;
        settings.setValue("terminal/preferred", preferredTerminal);
    });

    toolBar->addWidget(terminalSelector);
    addToolBar(Qt::TopToolBarArea, toolBar);

    // Main layout
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
    QMessageBox::information(this, tr("Assemble Result"), result);
    refreshContainers();
}

void MainWindow::updateButtonStates()
{
    bool hasSelection = !currentContainer.isEmpty();
    enterBtn->setEnabled(hasSelection);
    deleteBtn->setEnabled(hasSelection);
    appsBtn->setEnabled(hasSelection);
    upgradeBtn->setEnabled(hasSelection);

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

void MainWindow::refreshContainers()
{
    containerList->clear();
    QList<QMap<QString, QString>> containers = backend->getContainers();

    for (const auto &container : containers) {
        QListWidgetItem *item = new QListWidgetItem(container["name"], containerList);
        item->setData(Qt::UserRole + 3, container["image"]); // Store image name
        item->setData(Qt::UserRole + 4, container["distro"]); // Store distro name
        qDebug() << "Container:" << container["name"] << "Distro:" << container["distro"];
    }

    if (containers.isEmpty()) {
        QListWidgetItem *item = new QListWidgetItem("No containers found", containerList);
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        item->setIcon(QIcon::fromTheme("dialog-information"));
        qDebug() << "No containers found";
    }

    currentContainer.clear();
    updateButtonStates();
}

void MainWindow::deleteContainer()
{
    if (currentContainer.isEmpty())
        return;

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Confirm Deletion");
    msgBox.setText(QString("Delete container <b>%1</b>?").arg(currentContainer));
    msgBox.setInformativeText("This action cannot be undone.");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setIcon(QMessageBox::Warning);

    if (msgBox.exec() == QMessageBox::Yes) {
        QString result = backend->deleteContainer(currentContainer);
        QMessageBox::information(this, "Result", result);
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
        QProgressDialog progress("Creating container...", "Cancel", 0, 0, this);
        progress.setWindowModality(Qt::WindowModal);
        progress.show();

        QApplication::processEvents();

        QString result = backend->createContainer(dialog.containerName(), dialog.imageUrl(), dialog.homePath(), dialog.useInit(), dialog.volumes());

        progress.close();
        QMessageBox::information(this, "Result", result);
        refreshContainers();
    }
}

QString MainWindow::getContainerDistro() const
{
    QString distro = backend->getContainerDistro(currentContainer);
    return distro;
}

void MainWindow::installDebPackage()
{
    if (currentContainer.isEmpty())
        return;

    QString filePath = QFileDialog::getOpenFileName(this, "Select .deb Package", QDir::homePath(), "Debian Packages (*.deb)");
    if (!filePath.isEmpty()) {
        backend->installDebPackage(preferredTerminal, currentContainer, filePath);
    }
}

void MainWindow::installRpmPackage()
{
    if (currentContainer.isEmpty())
        return;

    QString filePath = QFileDialog::getOpenFileName(this, "Select .rpm Package", QDir::homePath(), "RPM Packages (*.rpm)");
    if (!filePath.isEmpty()) {
        backend->installRpmPackage(preferredTerminal, currentContainer, filePath);
    }
}

void MainWindow::installArchPackage()
{
    if (currentContainer.isEmpty())
        return;

    QString filePath = QFileDialog::getOpenFileName(this, "Select Arch Package", QDir::homePath(), "Arch Packages (*.pkg.tar.*)");
    if (!filePath.isEmpty()) {
        backend->installArchPackage(preferredTerminal, currentContainer, filePath);
    }
}

void MainWindow::assembleContainer()
{
    QString iniFile = QFileDialog::getOpenFileName(this, tr("Select Distrobox INI File"), QDir::homePath(), tr("INI Files (*.ini);;All Files (*)"));

    if (!iniFile.isEmpty()) {
        progressDialog = new QProgressDialog(tr("Assembling container..."), tr("Cancel"), 0, 0, this);
        progressDialog->setWindowModality(Qt::WindowModal);
        progressDialog->setCancelButton(nullptr); // Remove cancel button
        progressDialog->show();

        backend->assembleContainer(iniFile);
    }
}

void MainWindow::enterContainer()
{
    if (currentContainer.isEmpty())
        return;
    backend->enterContainer(currentContainer, preferredTerminal);
}

void MainWindow::upgradeContainer()
{
    if (currentContainer.isEmpty())
        return;
    backend->upgradeContainer(currentContainer, preferredTerminal);
}

void MainWindow::upgradeAllContainers()
{
    backend->upgradeAllContainers(preferredTerminal);
}
