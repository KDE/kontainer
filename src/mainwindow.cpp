#include "mainwindow.h"
#include "backend.h"
#include "createcontainerdialog.h"
#include <QTreeWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>
#include <QProgressDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), backend(new Backend(this))
{
    setWindowTitle("Kontainer");
    resize(800, 600);
    setupUI();
    refreshContainers();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    
    // Left panel - Container list
    QWidget *leftPanel = new QWidget(centralWidget);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    
    setupContainerList();
    leftLayout->addWidget(containerList);
    
    // Right panel - Action buttons
    QWidget *rightPanel = new QWidget(centralWidget);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    
    setupActionButtons();
    rightLayout->addWidget(availableAppsList);
    rightLayout->addWidget(exportedAppsList);
    
    mainLayout->addWidget(leftPanel, 1);
    mainLayout->addWidget(rightPanel, 1);
    setCentralWidget(centralWidget);
}

void MainWindow::setupContainerList()
{
    containerList = new QTreeWidget(this);
    containerList->setHeaderLabels({"Name", "Distro", "Status"});
    containerList->setContextMenuPolicy(Qt::CustomContextMenu);
    
    connect(containerList, &QTreeWidget::itemClicked, [this](QTreeWidgetItem *item) {
        currentContainer = item->text(0);
        showContainerDetails(currentContainer);
    });
    
    connect(containerList, &QTreeWidget::customContextMenuRequested,
            this, &MainWindow::showContainerContextMenu);
}

void MainWindow::setupActionButtons()
{
    availableAppsList = new QListWidget(this);
    availableAppsList->setSelectionMode(QAbstractItemView::SingleSelection);
    
    exportedAppsList = new QListWidget(this);
    exportedAppsList->setSelectionMode(QAbstractItemView::SingleSelection);
    
    QPushButton *exportBtn = new QPushButton("Export App", this);
    connect(exportBtn, &QPushButton::clicked, this, &MainWindow::exportSelectedApp);
    
    QPushButton *unexportBtn = new QPushButton("Unexport App", this);
    connect(unexportBtn, &QPushButton::clicked, this, &MainWindow::unexportSelectedApp);
    
    QPushButton *createBtn = new QPushButton("Create Container", this);
    connect(createBtn, &QPushButton::clicked, this, &MainWindow::createNewContainer);
    
    QPushButton *refreshBtn = new QPushButton("Refresh", this);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::refreshContainers);
}

void MainWindow::refreshContainers()
{
    containerList->clear();
    QList<QMap<QString, QString>> containers = backend->getContainers();
    
    for (const auto &container : containers) {
        QTreeWidgetItem *item = new QTreeWidgetItem({
            container["name"],
            container["distro"],
            container["status"]
        });
        containerList->addTopLevelItem(item);
    }
}

void MainWindow::showContainerContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = containerList->itemAt(pos);
    if (!item) return;
    
    currentContainer = item->text(0);
    QMenu menu(this);
    
    menu.addAction("Open Terminal", [this]() {
        backend->enterContainer(currentContainer);
    });
    
    menu.addAction("Upgrade Container", [this]() {
        backend->upgradeContainer(currentContainer);
    });
    
    menu.addAction("Delete Container", [this]() {
        if (QMessageBox::question(this, "Confirm", 
            "Delete container " + currentContainer + "?") == QMessageBox::Yes) {
            backend->deleteContainer(currentContainer);
            refreshContainers();
        }
    });
    
    menu.exec(containerList->mapToGlobal(pos));
}

void MainWindow::showContainerDetails(const QString &name)
{
    availableAppsList->clear();
    availableAppsList->addItem("Click Export to load apps");
    
    exportedAppsList->clear();
    exportedAppsList->addItem("Click Unexport to load apps");
}

void MainWindow::exportSelectedApp()
{
    if (currentContainer.isEmpty()) {
        QMessageBox::warning(this, "Error", "No container selected");
        return;
    }
    
    if (availableAppsList->count() == 1 && 
        availableAppsList->item(0)->text() == "Click Export to load apps") {
        availableAppsList->clear();
        availableAppsList->addItems(backend->getAvailableApps(currentContainer));
        return;
    }
    
    QString app = availableAppsList->currentItem()->text();
    QString result = backend->exportApp(app, currentContainer);
    QMessageBox::information(this, "Export App", result);
    exportedAppsList->clear();
}

void MainWindow::unexportSelectedApp()
{
    if (currentContainer.isEmpty()) {
        QMessageBox::warning(this, "Error", "No container selected");
        return;
    }
    
    if (exportedAppsList->count() == 1 && 
        exportedAppsList->item(0)->text() == "Click Unexport to load apps") {
        exportedAppsList->clear();
        exportedAppsList->addItems(backend->getExportedApps(currentContainer));
        return;
    }
    
    QString app = exportedAppsList->currentItem()->text();
    QString result = backend->unexportApp(app, currentContainer);
    QMessageBox::information(this, "Unexport App", result);
    exportedAppsList->clear();
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
