#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QTextStream>
#include <QColorGroup>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Cresemo)
{
    ui->setupUi(this);

    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    createFolderBorwser();
    disablePropertiesGroupBox();
}

void MainWindow::createFolderBorwser()
{
    dirModel.setFilter(QDir::AllDirs | QDir::Dirs | QDir::NoDotAndDotDot);

    ui->fileBrowser->setModel(&dirModel);

    ui->fileBrowser->setRootIndex(dirModel.index(QDir::rootPath()));
    ui->fileBrowser->setColumnHidden( 1, true );
    ui->fileBrowser->setColumnHidden( 2, true );
    ui->fileBrowser->setColumnHidden( 3, true );
}

void MainWindow::refreshDirectories()
{
    dirModel.refresh(dirModel.index(QDir::rootPath()));
}

void MainWindow::folderSelected(QModelIndex index)
{
    ui->fileBrowser->resizeColumnToContents(0);

    // Get the path of the selected index
    QString dirPath = dirModel.filePath(index);

    ui->currentFolder->setText(dirPath);

    if (isCVSFolder(dirPath) &&
	getCVSRootEntry(dirPath))
    {
	updateDefaultParameters();
        enablePropertiesGroupBox();
    }
    else
    {
	rootEntry = "";
	mServer = "";
	mUsername = "";
        disablePropertiesGroupBox();
    }

}

// Check if the given folder is a CVS folder...
bool MainWindow::isCVSFolder(const QString& path)
{
    // Add the CVS path
    QString cvsPath = path+"/CVS";
    QDir cvsDir(cvsPath);
    if (!cvsDir.exists())
        return false;
    return true;
}

bool MainWindow::getCVSRootEntry(const QString& path)
{
    // Add the CVS path
    QString rootPath = path+"/CVS/Root";
    QFile rootFile(rootPath);
    if (!rootFile.open(QIODevice::ReadOnly | QIODevice::Text))
	return false;

    QTextStream in(&rootFile);
    if (!in.atEnd())
    {
	rootEntry = in.readLine();
	return true;
    }
    else
	return false;
}

void MainWindow::updateDefaultParameters()
{
    // Update the parameters from the extracted root entry
    // Get index of @
    int indexOfNameServerDeparator = rootEntry.indexOf('@');
    // Get index of name start...
    int indexOfNameStart = rootEntry.lastIndexOf(':', indexOfNameServerDeparator);
    int indexOfServerEnd = rootEntry.lastIndexOf(':', -1);

    mUsername = rootEntry.mid(indexOfNameStart+1, indexOfNameServerDeparator-indexOfNameStart-1);
    mServer   = rootEntry.mid(indexOfNameServerDeparator+1, indexOfServerEnd-indexOfNameServerDeparator-1);
}

// Enables and initializes the groupbox
void MainWindow::enablePropertiesGroupBox()
{
    ui->cvsPropertiesGB->setEnabled(true);

    ui->usernameLE->setText(mUsername);
    ui->serverLE->setText(mServer);
}

// Disables Properties groupbox
void MainWindow::disablePropertiesGroupBox()
{
    ui->cvsPropertiesGB->setEnabled(false);
    ui->usernameLE->setText("");
    ui->serverLE->setText("");
    ui->recursiveCB->setChecked(true);
    ui->applyBN->setEnabled(false);
}

void MainWindow::usernameChanged(QString newUsername)
{
    if (newUsername != mUsername)
	changeLineEditBackgroundColor(ui->usernameLE, Qt::red);
    else
	changeLineEditBackgroundColor(ui->usernameLE, Qt::white);

    updateApplyButtonStatus();
}

void MainWindow::serverChanged(QString newServer)
{
    if (newServer != mServer)
	changeLineEditBackgroundColor(ui->serverLE, Qt::red);
    else
	changeLineEditBackgroundColor(ui->serverLE, Qt::white);

    updateApplyButtonStatus();
}

void MainWindow::updateApplyButtonStatus()
{
    if (ui->serverLE->text() != mServer ||
	ui->usernameLE->text() != mUsername)
	ui->applyBN->setEnabled(true);
    else
	ui->applyBN->setEnabled(false);
}

void MainWindow::changeLineEditBackgroundColor(QLineEdit* l, QColor C)
{
    QPalette palette(l->palette());
    palette.setColor(QPalette::Base, C);
    l->setPalette(palette);
}

void MainWindow::applyChanges()
{
    // Get new Entry
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QString newRootEntry = getNewRootEntry();

    QString cvsPath = ui->currentFolder->text();

    ui->infoWindow->appendPlainText("======== START UPDATING CVS ENTRIES ======");
    if (replaceRootEntries(cvsPath, newRootEntry))
    {
	// Update the rootEntry
	rootEntry = newRootEntry;
	updateDefaultParameters();
	usernameChanged(ui->usernameLE->text());
	serverChanged(ui->serverLE->text());

	ui->infoWindow->appendPlainText("======== FINISHED UPDATING CVS ENTRIES ======\n\n");
    }
    else
	ui->infoWindow->appendPlainText("======== SOME ERROR OCCURRED !!! ======\n\n");

    QApplication::restoreOverrideCursor();
}

QString MainWindow::getNewRootEntry()
{
    // add main execution code
    QString newRootEntry = rootEntry;
    QString usernameBefore = mUsername + "@";
    QString usernameAfter  = ui->usernameLE->text() + "@";
    QString serverBefore   = "@" + mServer;
    QString serverAfter    = "@" + ui->serverLE->text();
    newRootEntry.replace(usernameBefore, usernameAfter);
    newRootEntry.replace(serverBefore, serverAfter);

    return newRootEntry;
}

bool MainWindow::replaceRootEntries(QString path, QString rootEntry)
{

    // Non recurtsive
    if (ui->recursiveCB->isChecked() == false)
	return replaceRootEntry(path, rootEntry);

    bool replaceOK = true;

    // Get all subfolders
    QDir dir(path);
    dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);

    QFileInfoList list = dir.entryInfoList();

    for (int i = 0; i < list.size(); ++i)
    {
	QFileInfo fileInfo = list.at(i);
	QString subDirPath = fileInfo.absoluteFilePath();
	if (isCVSFolder(subDirPath))
	    replaceOK = replaceOK & replaceRootEntries(subDirPath, rootEntry);
    }

    replaceOK = replaceOK & replaceRootEntry(path, rootEntry);

    return replaceOK;
}

bool MainWindow::replaceRootEntry(QString path, QString rootEntry)
{
    QString rootPath = path+"/CVS/Root";
    QFile rootFile(rootPath);
    if (!rootFile.open(QIODevice::ReadOnly | QIODevice::Text))
	return false;

    // Remove previous root file
    rootFile.remove();

    // Create new file
    QFile ofile(rootPath);
    ofile.open(QIODevice::ReadWrite | QIODevice::Text);
    QTextStream out(&ofile);
    out << rootEntry;
    ofile.close();

    QString message = "Updated Root Entry in: "+ path;
    ui->infoWindow->appendPlainText(message);
    qApp->processEvents();
    return true;
}
