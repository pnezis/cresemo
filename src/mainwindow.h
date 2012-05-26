#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDirModel>
#include <QString>
#include <QLineEdit>

namespace Ui {
    class Cresemo;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void init();
    void createFolderBorwser();
    bool isCVSFolder(const QString& path);
    void enablePropertiesGroupBox();
    void disablePropertiesGroupBox();
    bool getCVSRootEntry(const QString& path);
    void updateDefaultParameters();

    void changeLineEditBackgroundColor(QLineEdit* l, QColor C);
    QString getNewRootEntry();
    bool replaceRootEntries(QString path, QString rootEntry);
    bool replaceRootEntry(QString path, QString rootEntry);
    void updateApplyButtonStatus();

private slots:
    void folderSelected(QModelIndex index);
    void usernameChanged(QString newName);
    void serverChanged(QString newServer);
    void applyChanges();
    void refreshDirectories();

private:
    Ui::Cresemo *ui;

    QDirModel dirModel;

    QString rootEntry;

    QString mUsername, mServer;
};

#endif // MAINWINDOW_H
