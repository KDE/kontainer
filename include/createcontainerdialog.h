#ifndef CREATECONTAINERDIALOG_H
#define CREATECONTAINERDIALOG_H

#include <QDialog>
#include <QProgressDialog>
#include <QProcess>

class Backend;
class QLineEdit;
class QListWidget;
class QCheckBox;

class CreateContainerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateContainerDialog(Backend *backend, QWidget *parent = nullptr);
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
