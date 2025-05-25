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


MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent), backend(new Backend(this))
{
    setWindowTitle("Kontainer");
    resize(400, 500);
    setupUI();
    refreshContainers();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    // Left panel - Container list
    containerList = new QListWidget(this);
    containerList->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(containerList, &QListWidget::itemSelectionChanged, [this]() {
        if (containerList->currentItem()) {
            currentContainer = containerList->currentItem()->text();
        }
    });

    // Right panel - Action buttons
    QWidget *rightPanel = new QWidget(centralWidget);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);

    enterBtn = new QPushButton("Enter", rightPanel);
    connect(enterBtn, &QPushButton::clicked, this, &MainWindow::enterContainer);

    deleteBtn = new QPushButton("Delete", rightPanel);
    connect(deleteBtn, &QPushButton::clicked, this, &MainWindow::deleteContainer);

    appsBtn = new QPushButton("Applications", rightPanel);
    connect(appsBtn, &QPushButton::clicked, this, &MainWindow::showAppsDialog);

    upgradeBtn = new QPushButton("Upgrade", rightPanel);
    connect(upgradeBtn, &QPushButton::clicked, this, &MainWindow::upgradeContainer);

    rightLayout->addWidget(enterBtn);
    rightLayout->addWidget(deleteBtn);
    rightLayout->addWidget(appsBtn);
    rightLayout->addWidget(upgradeBtn);
    rightLayout->addStretch();

    // Add button in toolbar
    addBtn = new QToolButton(this);
    addBtn->setIcon(QIcon::fromTheme("list-add"));
    addBtn->setToolTip("Create new container");
    connect(addBtn, &QToolButton::clicked, this, &MainWindow::createNewContainer);

    QToolBar *toolBar = new QToolBar(this);
    toolBar->addWidget(addBtn);
    addToolBar(toolBar);

    mainLayout->addWidget(containerList, 1);
    mainLayout->addWidget(rightPanel);
    setCentralWidget(centralWidget);
}

void MainWindow::refreshContainers()
{
    containerList->clear();
    QList<QMap<QString, QString>> containers = backend->getContainers();

    for (const auto &container : containers) {
        containerList->addItem(container["name"]);
    }
}

void MainWindow::enterContainer()
{
    if (currentContainer.isEmpty()) {
        QMessageBox::warning(this, "Error", "No container selected");
        return;
    }
    backend->enterContainer(currentContainer);
}

void MainWindow::deleteContainer()
{
    if (currentContainer.isEmpty()) {
        QMessageBox::warning(this, "Error", "No container selected");
        return;
    }

    if (QMessageBox::question(this, "Confirm",
        "Delete container " + currentContainer + "?") == QMessageBox::Yes) {
        backend->deleteContainer(currentContainer);
    refreshContainers();
        }
}

void MainWindow::showAppsDialog()
{
    if (currentContainer.isEmpty()) {
        QMessageBox::warning(this, "Error", "No container selected");
        return;
    }

    AppsDialog dialog(backend, currentContainer, this);
    dialog.exec();
}

void MainWindow::upgradeContainer()
{
    if (currentContainer.isEmpty()) {
        QMessageBox::warning(this, "Error", "No container selected");
        return;
    }
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
