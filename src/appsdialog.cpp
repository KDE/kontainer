// SPDX-License-Identifier: GPL-2.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2025 Hadi Chokr <hadichokr@icloud.com>

#include "appsdialog.h"
#include "backend.h"

// Custom item delegate for better looking list items
class AppListItemDelegate : public QStyledItemDelegate
{
public:
    explicit AppListItemDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        painter->save();

        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        if (opt.state & QStyle::State_Selected) {
            painter->fillRect(opt.rect, opt.palette.highlight());
        } else if (opt.state & QStyle::State_MouseOver) {
            QColor hoverColor = opt.palette.highlight().color();
            hoverColor.setAlpha(40);
            painter->fillRect(opt.rect, hoverColor);
        }

        int iconSize = opt.rect.height() - 8;
        QRect iconRect(opt.rect.x() + 4, opt.rect.y() + 4, iconSize, iconSize);
        QIcon icon = opt.icon;
        if (!icon.isNull()) {
            icon.paint(painter, iconRect, Qt::AlignCenter, opt.state & QStyle::State_Selected ? QIcon::Selected : QIcon::Normal);
        } else {
            QIcon fallback = QIcon::fromTheme("application-x-executable");
            fallback.paint(painter, iconRect, Qt::AlignCenter, opt.state & QStyle::State_Selected ? QIcon::Selected : QIcon::Normal);
        }

        painter->setPen(opt.state & QStyle::State_Selected ? opt.palette.highlightedText().color() : opt.palette.text().color());
        QRect textRect = opt.rect.adjusted(iconSize + 12, 0, -4, 0);
        QString elidedText = opt.fontMetrics.elidedText(opt.text, Qt::ElideRight, textRect.width());
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, elidedText);

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size.setHeight(qMax(size.height(), 36));
        return size;
    }
};

AppsDialog::AppsDialog(Backend *backend, const QString &containerName, QWidget *parent)
    : QDialog(parent)
    , m_backend(backend)
    , m_containerName(containerName)
{
    setWindowTitle(i18nc("@title:window, %1 is a container name", "Applications — %1", containerName));
    resize(600, 500);
    setWindowIcon(QIcon::fromTheme("applications-other"));

    m_tabs = new QTabWidget(this);
    m_tabs->setTabPosition(QTabWidget::North);
    m_tabs->setDocumentMode(true);
    m_tabs->setMovable(false);

    // Exported apps tab
    QWidget *exportedTab = new QWidget();
    QVBoxLayout *exportedLayout = new QVBoxLayout(exportedTab);
    exportedLayout->setContentsMargins(4, 4, 4, 4);
    exportedLayout->setSpacing(8);

    m_exportedAppsList = new QListWidget();
    m_exportedAppsList->setItemDelegate(new AppListItemDelegate(this));
    m_exportedAppsList->setIconSize(QSize(32, 32));
    m_exportedAppsList->setAlternatingRowColors(true);

    m_noExportedLabel = new QLabel(i18n("No exported applications"));
    m_noExportedLabel->setAlignment(Qt::AlignCenter);
    m_noExportedLabel->setVisible(false);

    m_unexportBtn = new QPushButton(i18n("Unexport"));
    m_unexportBtn->setIcon(QIcon::fromTheme("list-remove"));
    m_unexportBtn->setToolTip(i18n("Remove the selected application from your host system"));
    connect(m_unexportBtn, &QPushButton::clicked, [this]() {
        if (m_exportedAppsList->currentItem()) {
            QString app = m_exportedAppsList->currentItem()->text();
            QString result = m_backend->unexportApp(app, m_containerName);
            QMessageBox::information(this, i18n("Unexport App"), result);
            loadApps();
        }
    });

    exportedLayout->addWidget(m_exportedAppsList);
    exportedLayout->addWidget(m_noExportedLabel);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_unexportBtn);
    exportedLayout->addLayout(buttonLayout);

    // Available apps tab
    QWidget *availableTab = new QWidget();
    QVBoxLayout *availableLayout = new QVBoxLayout(availableTab);
    availableLayout->setContentsMargins(4, 4, 4, 4);
    availableLayout->setSpacing(8);

    m_availableAppsList = new QListWidget();
    m_availableAppsList->setItemDelegate(new AppListItemDelegate(this));
    m_availableAppsList->setIconSize(QSize(32, 32));
    m_availableAppsList->setAlternatingRowColors(true);

    m_noAvailableLabel = new QLabel(i18n("No available applications"));
    m_noAvailableLabel->setAlignment(Qt::AlignCenter);
    m_noAvailableLabel->setVisible(false);

    m_exportBtn = new QPushButton(i18n("Export"));
    m_exportBtn->setIcon(QIcon::fromTheme("list-add"));
    m_exportBtn->setToolTip(i18n("Make the selected application available on your host system"));
    connect(m_exportBtn, &QPushButton::clicked, [this]() {
        if (m_availableAppsList->currentItem()) {
            QString app = m_availableAppsList->currentItem()->text();
            QString result = m_backend->exportApp(app, m_containerName);
            QMessageBox::information(this, i18n("Export App"), result);
            loadApps();
        }
    });

    availableLayout->addWidget(m_availableAppsList);
    availableLayout->addWidget(m_noAvailableLabel);

    QHBoxLayout *exportButtonLayout = new QHBoxLayout();
    exportButtonLayout->addStretch();
    exportButtonLayout->addWidget(m_exportBtn);
    availableLayout->addLayout(exportButtonLayout);

    m_tabs->addTab(exportedTab, QIcon::fromTheme("emblem-shared"), i18n("Exported Apps"));
    m_tabs->addTab(availableTab, QIcon::fromTheme("applications-other"), i18n("Available Apps"));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->addWidget(m_tabs);

    loadApps();
}

void AppsDialog::loadApps()
{
    m_exportedAppsList->clear();
    QStringList exportedApps = m_backend->getExportedApps(m_containerName);
    for (const QString &app : exportedApps) {
        QIcon icon = QIcon::fromTheme(app.toLower());
        if (icon.isNull()) {
            icon = QIcon::fromTheme("package-x-generic");
        }
        QListWidgetItem *item = new QListWidgetItem(icon, app);
        m_exportedAppsList->addItem(item);
    }

    m_availableAppsList->clear();
    QStringList availableApps = m_backend->getAvailableApps(m_containerName);
    for (const QString &app : availableApps) {
        QIcon icon = QIcon::fromTheme(app.toLower());
        if (icon.isNull()) {
            icon = QIcon::fromTheme("package-x-generic");
        }
        QListWidgetItem *item = new QListWidgetItem(icon, app);
        m_availableAppsList->addItem(item);
    }

    // Sichtbarkeit & Platzhalter
    m_exportedAppsList->setVisible(!exportedApps.isEmpty());
    m_noExportedLabel->setVisible(exportedApps.isEmpty());
    m_unexportBtn->setVisible(!exportedApps.isEmpty());

    m_availableAppsList->setVisible(!availableApps.isEmpty());
    m_noAvailableLabel->setVisible(availableApps.isEmpty());
    m_exportBtn->setVisible(!availableApps.isEmpty());

    if (exportedApps.isEmpty() && m_tabs->currentIndex() == 0) {
        m_tabs->setCurrentIndex(1);
    }
}
