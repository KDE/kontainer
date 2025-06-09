#ifndef APPSDIALOG_H
#define APPSDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QListWidget>
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
