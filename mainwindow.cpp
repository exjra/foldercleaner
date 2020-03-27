#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QDebug>
#include <QFileDialog>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mTimer(nullptr),
    mDeleteTimer(nullptr),
    mDeleting(false)
{
    ui->setupUi(this);

    setWindowTitle("FolderCleaner v" + QCoreApplication::applicationVersion());

    QSettings tSettings;
    ui->lineEdit->setText(tSettings.value("lineEdit").toString());
    ui->checkBox->setChecked(tSettings.value("checkBox").toBool());
    ui->lineEdit_2->setText(tSettings.value("lineEdit_2").toString());
    ui->horizontalSlider->setValue(tSettings.value("horizontalSlider").toInt());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_2_clicked()
{
    if(mTimer == nullptr)
    {
        mTimer = new QTimer();
        connect(mTimer, &QTimer::timeout, this, &MainWindow::onTimerTick);
        mTimer->setInterval(ui->horizontalSlider->value()*1000);
        mTimer->start();

        ui->pushButton_2->setText("Durdur");

        mDeleteTimer = new QTimer();
        connect(mDeleteTimer, &QTimer::timeout, this, &MainWindow::onDeleteTick);
        mDeleteTimer->setInterval(1000);
        mDeleteTimer->start();

        ui->lineEdit->setEnabled(false);
        ui->pushButton->setEnabled(false);
        ui->frame->setEnabled(false);
        ui->horizontalSlider->setEnabled(false);

        QSettings tSettings;
        tSettings.setValue("lineEdit", ui->lineEdit->text());
        tSettings.setValue("checkBox", ui->checkBox->isChecked());
        tSettings.setValue("lineEdit_2", ui->lineEdit_2->text());
        tSettings.setValue("horizontalSlider", ui->horizontalSlider->value());
    }
    else {
        mTimer->stop();
        disconnect(mTimer, nullptr, nullptr, nullptr);
        mTimer->deleteLater();
        mTimer = nullptr;

        ui->pushButton_2->setText("Başlat");

        mDeleteTimer->stop();
        disconnect(mDeleteTimer, nullptr, nullptr, nullptr);
        mDeleteTimer->deleteLater();
        mDeleteTimer = nullptr;

        ui->lineEdit->setEnabled(true);
        ui->pushButton->setEnabled(true);
        ui->frame->setEnabled(true);
        ui->horizontalSlider->setEnabled(true);
    }
}

void MainWindow::onTimerTick()
{
    if(ui->lineEdit->text() == "")
    {
        addLog("Hata! Klasör belirtiniz");
        return;
    }

    //assume the directory exists and contains some files and you want all jpg and JPG files
    QDir directory(ui->lineEdit->text());
    if(!directory.exists())
    {
        addLog("Hata! Klasör bulunamadı");
        return;
    }

    QStringList tListedFiles;
    tListedFiles = getFilesUnderDir(ui->lineEdit->text());

    QStringList tDirs = directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach(QString dir, tDirs)
        tListedFiles += getFilesUnderDir(ui->lineEdit->text() + QDir::separator() + dir);

    foreach(QString filename, tListedFiles) {
        if(!mFiles.contains(filename))
        {
            mFiles.append(filename);
            addLog("Bulunan dosya: " + filename);
        }
    }
}

QStringList MainWindow::getFilesUnderDir(QString pDirPath)
{
    QDir directory(pDirPath);
    if(!directory.exists())
    {
        addLog("Hata! Klasör bulunamadı :" + pDirPath);
        return QStringList();
    }

    QStringList tListedReturn;

    if(ui->checkBox->isChecked())
    {
        QStringList tListedFiles = directory.entryList(QDir::Files);
        foreach(QString tfile, tListedFiles)
            tListedReturn.append(pDirPath + QDir::separator() + tfile);
    }
    else
    {
        if(ui->lineEdit_2->text() == "")
        {
            addLog("Silinmesini istediğiniz dosya uzantılarını belirtiniz\n*.mov,*.jpg,*.xyz gibi aralarında boşluk bırakmadan virgül ile ayırarak belirtiniz");
            return QStringList();
        }
        else
        {
            QStringList tListedFiles = directory.entryList(ui->lineEdit_2->text().split(","), QDir::Files);
            foreach(QString tfile, tListedFiles)
                tListedReturn.append(pDirPath + QDir::separator() + tfile);
        }
    }

    return tListedReturn;
}

void MainWindow::on_pushButton_clicked()
{
    QString tTemp = QFileDialog::getExistingDirectory();
    if(tTemp != "")
        ui->lineEdit->setText(tTemp);
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    ui->label_3->setText(QString::number(value) + " saniye");
}

void MainWindow::onDeleteTick()
{
    if(mDeleting)
        return;

    if(mFiles.length() > 0)
    {
        QFile tFile(mFiles.takeFirst());
        if(!tFile.exists())
        {
            addLog("Hata! Dosya listeye eklenmiş ancak silerken bulunamadı : " + tFile.fileName());
            return;
        }

        if(tFile.remove())
            addLog("Dosya silindi: " + tFile.fileName());
        else
            addLog("Hata! Dosya silinemedi: " + tFile.fileName());
    }
}

void MainWindow::addLog(QString pStr)
{
    ui->textEdit->setPlainText(pStr + "\n" + ui->textEdit->toPlainText());
}
