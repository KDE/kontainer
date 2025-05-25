#include "appsdialog.h"
#include "backend.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QIcon>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QApplication>
#include <QStyle>
#include <QFontMetrics>

// Custom item delegate for better looking list items
class AppListItemDelegate : public QStyledItemDelegate {
public:
    explicit AppListItemDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
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

        // Draw icon
        int iconSize = opt.rect.height() - 8;
        QRect iconRect(opt.rect.x() + 4, opt.rect.y() + 4, iconSize, iconSize);
        QIcon icon = opt.icon;
        if (!icon.isNull()) {
            icon.paint(painter, iconRect, Qt::AlignCenter,
                       opt.state & QStyle::State_Selected ? QIcon::Selected : QIcon::Normal);
        }

        // Draw text
        painter->setPen(opt.state & QStyle::State_Selected ?
        opt.palette.highlightedText().color() :
        opt.palette.text().color());
        QRect textRect = opt.rect.adjusted(iconSize + 12, 0, -4, 0);
        QString elidedText = opt.fontMetrics.elidedText(opt.text, Qt::ElideRight, textRect.width());
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, elidedText);

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size.setHeight(qMax(size.height(), 36)); // Minimum row height
        return size;
    }
};

AppsDialog::AppsDialog(Backend *backend, const QString &containerName, QWidget *parent)
: QDialog(parent), m_backend(backend), m_containerName(containerName)
{
    // Window setup
    setWindowTitle("Applications - " + containerName);
    resize(600, 500);
    setWindowIcon(QIcon::fromTheme("applications-other"));

    // Apply modern style
    setStyleSheet(R"(
        QDialog {
            background: palette(window);
        }
        QTabWidget::pane {
            border-top: 1px solid palette(mid);
        }
        QTabBar::tab {
            padding: 8px 16px;
            border: 1px solid palette(mid);
            border-bottom: none;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
            margin-right: 4px;
            background: palette(button);
        }
        QTabBar::tab:selected {
            background: palette(window);
            border-bottom: 1px solid palette(window);
            margin-bottom: -1px;
        }
        QListWidget {
            border: 1px solid palette(mid);
            border-radius: 4px;
            background: palette(base);
            alternate-background-color: palette(alternate-base);
        }
        QPushButton {
            padding: 6px 12px;
            border-radius: 4px;
            min-width: 80px;
        }
    )");

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

    QPushButton *unexportBtn = new QPushButton("Unexport");
    unexportBtn->setIcon(QIcon::fromTheme("list-remove"));
    connect(unexportBtn, &QPushButton::clicked, [this]() {
        if (m_exportedAppsList->currentItem()) {
            QString app = m_exportedAppsList->currentItem()->text();
            QString result = m_backend->unexportApp(app, m_containerName);
            QMessageBox::information(this, "Unexport App", result);
            loadApps();
        }
    });

    exportedLayout->addWidget(m_exportedAppsList);
    exportedLayout->addWidget(unexportBtn, 0, Qt::AlignRight);

    // Available apps tab
    QWidget *availableTab = new QWidget();
    QVBoxLayout *availableLayout = new QVBoxLayout(availableTab);
    availableLayout->setContentsMargins(4, 4, 4, 4);
    availableLayout->setSpacing(8);

    m_availableAppsList = new QListWidget();
    m_availableAppsList->setItemDelegate(new AppListItemDelegate(this));
    m_availableAppsList->setIconSize(QSize(32, 32));
    m_availableAppsList->setAlternatingRowColors(true);

    QPushButton *exportBtn = new QPushButton("Export");
    exportBtn->setIcon(QIcon::fromTheme("list-add"));
    connect(exportBtn, &QPushButton::clicked, [this]() {
        if (m_availableAppsList->currentItem()) {
            QString app = m_availableAppsList->currentItem()->text();
            QString result = m_backend->exportApp(app, m_containerName);
            QMessageBox::information(this, "Export App", result);
            loadApps();
        }
    });

    availableLayout->addWidget(m_availableAppsList);
    availableLayout->addWidget(exportBtn, 0, Qt::AlignRight);

    m_tabs->addTab(exportedTab, QIcon::fromTheme("emblem-shared"), "Exported Apps");
    m_tabs->addTab(availableTab, QIcon::fromTheme("applications-other"), "Available Apps");

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
        QListWidgetItem *item = new QListWidgetItem(QIcon::fromTheme(app.toLower()), app);
        m_exportedAppsList->addItem(item);
    }

    m_availableAppsList->clear();
    QStringList availableApps = m_backend->getAvailableApps(m_containerName);
    for (const QString &app : availableApps) {
        QListWidgetItem *item = new QListWidgetItem(QIcon::fromTheme(app.toLower()), app);
        m_availableAppsList->addItem(item);
    }
}
