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
        QString distro = index.data(Qt::UserRole + 1).toString();
        bool isRunning = index.data(Qt::UserRole + 2).toString().contains("running", Qt::CaseInsensitive);

        // Draw icon using system theme's box icon
        int iconSize = opt.rect.height() - 8;
        QRect iconRect(opt.rect.x() + 4, opt.rect.y() + 4, iconSize, iconSize);

        QIcon icon = QIcon::fromTheme("package-x-generic"); // System theme box icon
        if (isRunning) {
            icon.paint(painter, iconRect, Qt::AlignCenter, QIcon::Normal);
        } else {
            icon.paint(painter, iconRect, Qt::AlignCenter, QIcon::Disabled);
        }

        // Draw text with accent color
        QColor accentColor = getDistroColor(distro);
        if (opt.state & QStyle::State_Selected) {
            painter->setPen(opt.palette.highlightedText().color());
        } else {
            painter->setPen(accentColor);
        }

        QRect textRect = opt.rect.adjusted(iconSize + 12, 0, -30, 0);
        QString elidedText = opt.fontMetrics.elidedText(opt.text, Qt::ElideRight, textRect.width());
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, elidedText);

        // Simple status indicator
        painter->setPen(Qt::NoPen);
        painter->setBrush(isRunning ? QColor(46, 204, 113) : QColor(231, 76, 60));
        painter->drawEllipse(opt.rect.right() - 16, opt.rect.y() + (opt.rect.height() - 6)/2, 6, 6);

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size.setHeight(qMax(size.height(), 36)); // Slightly smaller row height
        return size;
    }

private:
    QColor getDistroColor(const QString &distro) const {
        static QMap<QString, QColor> distroColors = {
            {"ubuntu", QColor(233, 84, 32)},    // Ubuntu orange
            {"debian", QColor(211, 32, 42)},    // Debian red
            {"fedora", QColor(52, 101, 164)},   // Fedora blue
            {"arch", QColor(23, 147, 209)},     // Arch blue
            {"centos", QColor(132, 28, 45)},    // CentOS red
            {"alpine", QColor(10, 108, 155)},   // Alpine blue
            {"gentoo", QColor(102, 2, 60)},     // Gentoo purple
            {"opensuse", QColor(111, 180, 36)},  // openSUSE green
            {"void", QColor(72, 129, 52)}       // Void green
        };

        return distroColors.value(distro.toLower(), QColor(100, 100, 100)); // Default gray
    }
};

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent), backend(new Backend(this))
{
    setWindowTitle("Kontainer");
    resize(600, 600);
    setWindowIcon(QIcon::fromTheme("package-x-generic"));

    // Apply modern styling
    setStyleSheet(R"(
        QMainWindow {
            background: palette(window);
        }
        QListWidget {
            border: 1px solid palette(mid);
            border-radius: 4px;
            background: palette(base);
            alternate-background-color: palette(alternate-base);
        }
        QPushButton {
            padding: 10px 16px;
            border-radius: 4px;
            min-width: 120px;
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                      stop:0 palette(button), stop:1 palette(dark));
            color: palette(button-text);
            border: 1px solid palette(shadow);
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                      stop:0 palette(light), stop:1 palette(button));
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                      stop:0 palette(dark), stop:1 palette(button));
        }
        QToolButton {
            padding: 6px;
            border-radius: 4px;
            background: transparent;
        }
        QToolButton:hover {
            background: rgba(0,0,0,20);
        }
    )");

    setupUI();
    refreshContainers();
}

MainWindow::~MainWindow() {}

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

    // Add button in toolbar
    QToolBar *toolBar = new QToolBar(this);
    toolBar->setMovable(false);
    toolBar->setIconSize(QSize(24, 24));

    addBtn = new QToolButton(toolBar);
    addBtn->setIcon(QIcon::fromTheme("list-add"));
    addBtn->setText("New Container");
    addBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    addBtn->setToolTip("Create new container");
    connect(addBtn, &QToolButton::clicked, this, &MainWindow::createNewContainer);

    toolBar->addWidget(addBtn);
    addToolBar(Qt::TopToolBarArea, toolBar);

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
        item->setData(Qt::UserRole + 1, container["distro"]); // Store distro
        item->setData(Qt::UserRole + 2, container["status"]); // Store status
    }

    if (containers.isEmpty()) {
        QListWidgetItem *item = new QListWidgetItem("No containers found", containerList);
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        item->setIcon(QIcon::fromTheme("dialog-information"));
    }

    currentContainer.clear();
    updateButtonStates();
}

void MainWindow::enterContainer()
{
    if (currentContainer.isEmpty()) return;
    backend->enterContainer(currentContainer);
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

void MainWindow::upgradeContainer()
{
    if (currentContainer.isEmpty()) return;
    backend->upgradeContainer(currentContainer);
}

void MainWindow::createNewContainer()
{
    CreateContainerDialog dialog(backend, this);
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
