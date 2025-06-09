#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QPainter>
#include <QProgressDialog>
#include <QPushButton>
#include <QSettings>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QTabWidget>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <cctype>

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
    void assembleContainer();
    void onAssembleFinished(const QString &result);
    void installDebPackage();
    void installRpmPackage();
    void installArchPackage();

private:
    void setupUI();
    void setupContainerList();
    void setupActionButtons();
    void showAppsForContainer(const QString &name);
    void updateButtonStates();
    QToolButton *assembleBtn;
    QProgressDialog *progressDialog = nullptr;
    QPushButton *installDebBtn;
    QPushButton *installRpmBtn;
    QPushButton *installArchBtn;
    QString getContainerDistro() const;

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
