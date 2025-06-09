#ifndef CREATECONTAINERDIALOG_H
#define CREATECONTAINERDIALOG_H

#include <QApplication>
#include <QCheckBox>
#include <QDebug>
#include <QDialog>
#include <QFontMetrics>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPainter>
#include <QProcess>
#include <QProgressDialog>
#include <QPushButton>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

class Backend;
class QLineEdit;
class QListWidget;
class QCheckBox;

class CreateContainerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateContainerDialog(Backend *backend, const QString &terminal, QWidget *parent = nullptr);
    ~CreateContainerDialog();

    QString containerName() const;
    QString imageUrl() const;
    QString homePath() const;
    bool useInit() const;
    QStringList volumes() const;

private slots:
    void refreshImages();
    void searchImages(const QString &query);
    void startContainerCreation();
    void handleCreateFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void handleReadyRead();
    void handleErrorOccurred(QProcess::ProcessError error);

private:
    Backend *m_backend;
    QString m_terminal;
    QLineEdit *m_nameEdit;
    QLineEdit *m_searchEdit;
    QListWidget *m_imageList;
    QLineEdit *m_homeEdit;
    QLineEdit *m_volumesEdit;
    QCheckBox *m_initCheckbox;
    QProgressDialog *m_progressDialog;
    QProcess *m_createProcess;
};

#endif // CREATECONTAINERDIALOG_H
