#include "appsdialog.h"
#include "backend.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>

AppsDialog::AppsDialog(Backend *backend, const QString &containerName, QWidget *parent)
: QDialog(parent), m_backend(backend), m_containerName(containerName)
{
    setWindowTitle("Applications - " + containerName);
    resize(500, 400);

    m_tabs = new QTabWidget(this);

    // Exported apps tab
    QWidget *exportedTab = new QWidget();
    QVBoxLayout *exportedLayout = new QVBoxLayout(exportedTab);
    m_exportedAppsList = new QListWidget();
    QPushButton *unexportBtn = new QPushButton("Unexport");
    connect(unexportBtn, &QPushButton::clicked, [this]() {
        if (m_exportedAppsList->currentItem()) {
            QString app = m_exportedAppsList->currentItem()->text();
            QString result = m_backend->unexportApp(app, m_containerName);
            QMessageBox::information(this, "Unexport App", result);
            loadApps();
        }
    });
    exportedLayout->addWidget(m_exportedAppsList);
    exportedLayout->addWidget(unexportBtn);
    m_tabs->addTab(exportedTab, "Exported Apps");

    // Available apps tab
    QWidget *availableTab = new QWidget();
    QVBoxLayout *availableLayout = new QVBoxLayout(availableTab);
    m_availableAppsList = new QListWidget();
    QPushButton *exportBtn = new QPushButton("Export");
    connect(exportBtn, &QPushButton::clicked, [this]() {
        if (m_availableAppsList->currentItem()) {
            QString app = m_availableAppsList->currentItem()->text();
            QString result = m_backend->exportApp(app, m_containerName);
            QMessageBox::information(this, "Export App", result);
            loadApps();
        }
    });
    availableLayout->addWidget(m_availableAppsList);
    availableLayout->addWidget(exportBtn);
    m_tabs->addTab(availableTab, "Available Apps");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_tabs);

    loadApps();
}

void AppsDialog::loadApps()
{
    m_exportedAppsList->clear();
    m_exportedAppsList->addItems(m_backend->getExportedApps(m_containerName));

    m_availableAppsList->clear();
    m_availableAppsList->addItems(m_backend->getAvailableApps(m_containerName));
}
