#ifndef CREATECONTAINERDIALOG_H
#define CREATECONTAINERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QListWidget>
#include <QFormLayout>

class Backend;

class CreateContainerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CreateContainerDialog(Backend *backend, QWidget *parent = nullptr);

    QString containerName() const;
    QString imageUrl() const;
    QString homePath() const;
    bool useInit() const;
    QStringList volumes() const;

private slots:
    void refreshImages();
    void searchImages(const QString &query);

private:
    Backend *m_backend;
    QLineEdit *m_nameEdit;
    QLineEdit *m_homeEdit;
    QLineEdit *m_volumesEdit;
    QCheckBox *m_initCheckbox;
    QLineEdit *m_searchEdit;
    QListWidget *m_imageList;
};

#endif // CREATECONTAINERDIALOG_H