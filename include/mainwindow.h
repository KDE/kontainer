#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QTabWidget>
#include <QToolButton>

class Backend;
class QListWidget;
class QPushButton;
class CreateContainerDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QString preferredTerminal;

private slots:
    void refreshContainers();
    void enterContainer();
    void deleteContainer();
    void showAppsDialog();
    void upgradeContainer();
    void upgradeAllContainers();
    void createNewContainer();

private:
    void setupUI();
    void setupContainerList();
    void setupActionButtons();
    void showAppsForContainer(const QString &name);
    void updateButtonStates();


    Backend *backend;
    QListWidget *containerList;
    QPushButton *enterBtn;
    QPushButton *deleteBtn;
    QPushButton *appsBtn;
    QPushButton *upgradeBtn;
    QToolButton *addBtn;
    QToolButton *aBtn;
    QString currentContainer;
};

#endif // MAINWINDOW_H
