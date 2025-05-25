#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QApplication> 

class Backend;
class QTreeWidget;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void refreshContainers();
    void showContainerContextMenu(const QPoint &pos);
    void exportSelectedApp();
    void unexportSelectedApp();
    void createNewContainer();

private:
    void setupUI();
    void setupContainerList();
    void setupActionButtons();
    void showContainerDetails(const QString &name);
    
    Backend *backend;
    QTreeWidget *containerList;
    QListWidget *availableAppsList;
    QListWidget *exportedAppsList;
    QString currentContainer;
};

#endif // MAINWINDOW_H
