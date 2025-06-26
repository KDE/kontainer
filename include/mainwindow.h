// SPDX-License-Identifier: GPL-2.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2025 Hadi Chokr <hadichokr@icloud.com>

#pragma once

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
#include <QProgressBar>
#include <QProgressDialog>
#include <QPushButton>
#include <QSettings>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QTabWidget>
#include <QTextEdit>
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
    void showCommandOutput(const QString &output);
    QString preferredBackend;

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
    void onBackendsAvailable(const QStringList &backends);

private:
    void setupUI();
    void setupLoadingUI();
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
    bool hasTerminal = false;
    QTextEdit *progressOutput;
    void setupProgressDialog(const QString &title);
    void appendCommandOutput(const QString &output);

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
