#include "mainwindow.h"
#include "backend.h"
#include "createcontainerdialog.h"
#include "appsdialog.h"
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QMessageBox>
#include <QProgressDialog>
#include <QApplication>
#include <QIcon>
#include <QToolBar>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyle>
#include <QFontMetrics>
#include <cctype>
#include <QComboBox>
#include <QSettings>
#include <QLabel>



// Custom delegate for container list items
class ContainerItemDelegate : public QStyledItemDelegate {
public:
    explicit ContainerItemDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
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
        QString image = index.data(Qt::UserRole + 3).toString(); // Get image name

        // Draw icon using system theme's box icon
        int iconSize = opt.rect.height() - 8;
        QRect iconRect(opt.rect.x() + 4, opt.rect.y() + 4, iconSize, iconSize);

        QIcon icon = QIcon::fromTheme("package-x-generic");
        icon.paint(painter, iconRect, Qt::AlignCenter, QIcon::Normal);

        // Draw container name with accent color
        painter->setPen(opt.state & QStyle::State_Selected ?
        opt.palette.highlightedText().color() :
        opt.palette.text().color());

        QRect nameRect = opt.rect.adjusted(iconSize + 12, 0, -30, -opt.rect.height()/2);
        QString elidedName = opt.fontMetrics.elidedText(opt.text, Qt::ElideRight, nameRect.width());
        painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, elidedName);

        // Draw image name below
        QFont smallFont = opt.font;
        smallFont.setPointSize(smallFont.pointSize() - 2);
        painter->setFont(smallFont);

        QColor textColor = opt.palette.text().color();
        textColor.setAlpha(180); // Slightly muted color
        painter->setPen(textColor);

        QRect imageRect = opt.rect.adjusted(iconSize + 12, opt.rect.height()/2, -30, 0);
        QString elidedImage = opt.fontMetrics.elidedText(image, Qt::ElideRight, imageRect.width());
        painter->drawText(imageRect, Qt::AlignLeft | Qt::AlignVCenter, elidedImage);

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size.setHeight(qMax(size.height(), 48)); // Enough space for two lines
        return size;
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), backend(new Backend(this)), preferredTerminal("xterm")
{
    // Load saved terminal preference
    QSettings settings;
    preferredTerminal = settings.value("terminal/preferred", "xterm").toString();

    setWindowTitle("Kontainer");
    resize(600, 600);
    setWindowIcon(QIcon::fromTheme("package-x-generic"));

    setupUI();
    refreshContainers();
}

MainWindow::~MainWindow() {
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

    // Add expanding spacer between left and right sections
    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolBar->addWidget(spacer);

    // Right side - terminal selection
    QLabel *terminalLabel = new QLabel("Terminal:", toolBar);
    toolBar->addWidget(terminalLabel);

    QComboBox *terminalSelector = new QComboBox(toolBar);
    QStringList terminalOptions = {"gnome-terminal", "konsole", "xfce4-terminal", "xterm"};

    for (const QString &term : terminalOptions) {
        if (!QStandardPaths::findExecutable(term).isEmpty()) {
            terminalSelector->addItem(term);
        }
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
    });

    toolBar->addWidget(terminalSelector);
    addToolBar(Qt::TopToolBarArea, toolBar);

    // Main layout
    mainLayout->addWidget(containerList, 1);
    mainLayout->addWidget(rightPanel);
    setCentralWidget(centralWidget);

    updateButtonStates();
}

void MainWindow::updateButtonStates()
{
    bool hasSelection = !currentContainer.isEmpty();
    enterBtn->setEnabled(hasSelection);
    deleteBtn->setEnabled(hasSelection);
    appsBtn->setEnabled(hasSelection);
    upgradeBtn->setEnabled(hasSelection);
}

void MainWindow::refreshContainers()
{
    containerList->clear();
    QList<QMap<QString, QString>> containers = backend->getContainers();

    for (const auto &container : containers) {
        QListWidgetItem *item = new QListWidgetItem(container["name"], containerList);
        item->setData(Qt::UserRole + 3, container["image"]); // Store image name
    }

    if (containers.isEmpty()) {
        QListWidgetItem *item = new QListWidgetItem("No containers found", containerList);
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        item->setIcon(QIcon::fromTheme("dialog-information"));
    }

    currentContainer.clear();
    updateButtonStates();
}

void MainWindow::deleteContainer()
{
    if (currentContainer.isEmpty()) return;

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
    if (currentContainer.isEmpty()) return;

    AppsDialog dialog(backend, currentContainer, this);
    dialog.exec();
}

void MainWindow::createNewContainer()
{
    CreateContainerDialog dialog(backend, preferredTerminal, this);
    if (dialog.exec() == QDialog::Accepted) {
        QProgressDialog progress("Creating container...", "Cancel", 0, 0, this);
        progress.setWindowModality(Qt::WindowModal);
        progress.show();

        QApplication::processEvents();

        QString result = backend->createContainer(
            dialog.containerName(),
                                                  dialog.imageUrl(),
                                                  dialog.homePath(),
                                                  dialog.useInit(),
                                                  dialog.volumes()
        );

        progress.close();
        QMessageBox::information(this, "Result", result);
        refreshContainers();
    }
}

void MainWindow::enterContainer()
{
    if (currentContainer.isEmpty()) return;
    backend->enterContainer(currentContainer, preferredTerminal);
}

void MainWindow::upgradeContainer()
{
    if (currentContainer.isEmpty()) return;
    backend->upgradeContainer(currentContainer, preferredTerminal);
}

void MainWindow::upgradeAllContainers()
{
    backend->upgradeAllContainers(preferredTerminal);
}

