#ifndef APPSDIALOG_H
#define APPSDIALOG_H

#include <QApplication>
#include <QDialog>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QIcon>
#include <QListWidget>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QTabWidget>
#include <QVBoxLayout>

class Backend;

class AppsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AppsDialog(Backend *backend, const QString &containerName, QWidget *parent = nullptr);

private:
    void loadApps();

    Backend *m_backend;
    QString m_containerName;
    QTabWidget *m_tabs;
    QListWidget *m_exportedAppsList;
    QListWidget *m_availableAppsList;
};

#endif // APPSDIALOG_H
