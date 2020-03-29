#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QDebug>
#include <QFileDialog>
#include <QSettings>
#include <QDateTime>
#include <QProcess>
#include <QThread>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mTimer(nullptr),
    mDeleteTimer(nullptr),
    mDeleting(false),
    mConverter(nullptr)
{
    ui->setupUi(this);

    setWindowTitle("FolderCleaner v" + QCoreApplication::applicationVersion());

    QSettings tSettings;
    ui->lineEdit->setText(tSettings.value("lineEdit").toString());
    ui->checkBox->setChecked(tSettings.value("checkBox").toBool());
    ui->checkBox_2->setChecked(tSettings.value("checkBox_2").toBool());
    ui->lineEdit_2->setText(tSettings.value("lineEdit_2").toString());
    ui->lineEdit_3->setText(tSettings.value("lineEdit_3").toString());
    ui->lineEdit_4->setText(tSettings.value("lineEdit_4").toString());
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
        mFiles.clear();

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
        ui->lineEdit_3->setEnabled(false);
        ui->lineEdit_4->setEnabled(false);
        ui->pushButton->setEnabled(false);
        ui->checkBox_2->setEnabled(false);
        ui->frame->setEnabled(false);
        ui->horizontalSlider->setEnabled(false);

        QSettings tSettings;
        tSettings.setValue("lineEdit", ui->lineEdit->text());
        tSettings.setValue("checkBox", ui->checkBox->isChecked());
        tSettings.setValue("checkBox_2", ui->checkBox_2->isChecked());
        tSettings.setValue("lineEdit_2", ui->lineEdit_2->text());
        tSettings.setValue("lineEdit_3", ui->lineEdit_3->text());
        tSettings.setValue("lineEdit_4", ui->lineEdit_4->text());
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
        ui->lineEdit_3->setEnabled(true);
        ui->checkBox_2->setEnabled(true);
        ui->lineEdit_4->setEnabled(true);
        ui->pushButton->setEnabled(true);
        ui->frame->setEnabled(true);
        ui->horizontalSlider->setEnabled(true);
    }
}

void MainWindow::onTimerTick()
{
    if(ui->lineEdit->text() == "")
    {
        addLog("Hata! Klasör belirtiniz.");
        return;
    }

    if(ui->checkBox_2->isChecked())
    {
        if(ui->lineEdit_3->text() == "")
        {
            addLog("Hata! FFMpeg program yolunu belirtiniz.");
            return;
        }
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

void MainWindow::processOutput()
{
    QProcess *p = qobject_cast<QProcess*>(sender());
    p->setReadChannel(QProcess::StandardOutput);
    while(p->canReadLine())
        ui->textEdit_2->setPlainText(p->readLine());
}

void MainWindow::processError()
{
    QProcess *p = qobject_cast<QProcess*>(sender());
    p->setReadChannel(QProcess::StandardError);
    while(p->canReadLine())
        ui->textEdit_2->setPlainText(p->readLine());
}

void MainWindow::processFinished(int pCode)
{
    addLog("Process Finished.");
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
        QString tPath = mFiles.first();
        QFile tFile(tPath);
        if(!tFile.exists())
        {
            addLog("Hata! Dosya listeye eklenmiş ancak silerken bulunamadı : " + tFile.fileName());
            return;
        }

        if(!ui->checkBox_2->isChecked())
        {
            QFileInfo tFileInfo(tFile);

            if(tFileInfo.isReadable() &&
                    tFileInfo.isWritable() &&
                    !tFileInfo.isSymLink() &&
                    (tFileInfo.lastModified() < QDateTime::currentDateTime().addSecs(-10)))
            {
                mFiles.takeFirst();
                if(tFile.remove())
                    addLog("Dosya silindi: " + tFile.fileName());
                else
                    addLog("Hata! Dosya silinemedi: " + tFile.fileName());
            }
        }
        else {
            QFileInfo tFileInfo(tFile);

            if(tFileInfo.isReadable() &&
                    tFileInfo.isWritable() &&
                    !tFileInfo.isSymLink() &&
                    (tFileInfo.lastModified() < QDateTime::currentDateTime().addSecs(-10)))
            {
                //            addLog("Bok:" + tFileInfo.filePath() + " " + tFileInfo.baseName() + " " + tFileInfo.suffix() + ":" + tFileInfo.dir().path());

//                QThread* tThread = new QThread;
                QProcess* mConverter = new QProcess();

                connect(mConverter, &QProcess::readyReadStandardOutput, this, &MainWindow::processOutput, Qt::DirectConnection);
                connect(mConverter, &QProcess::readyReadStandardError, this, &MainWindow::processError, Qt::DirectConnection);
                connect(mConverter, SIGNAL(finished(int)), this, SLOT(processFinished(int)));

//                mConverter->moveToThread(tThread);

                mConverter->setProgram(ui->lineEdit_3->text() + QDir::separator() + "ffmpeg.exe");
                mConverter->setArguments(QStringList()
                                         << "-i"
                                         << tFileInfo.filePath()
                                         << ui->lineEdit_4->text().split(" ")
                                         << tFileInfo.dir().path() + QDir::separator() + tFileInfo.baseName() + ".mp4");

                mConverter->start(QIODevice::ReadWrite);

                mConverter->waitForStarted();
                addLog("Coverter basladi: " + tFileInfo.baseName() + ".mov -> " + tFileInfo.baseName() + ".mp4");
                addLog("Coverter Parametreleri: " + ui->lineEdit_4->text());

                while(mConverter->state() == QProcess::ProcessState::Running || mConverter->state() == QProcess::ProcessState::Starting)
                {
                    qApp->processEvents();

//                    QFileInfo tTargetFile(tFileInfo.dir().path() + QDir::separator() + tFileInfo.baseName() + ".mp4");

//                    if(tTargetFile.exists())
//                    {
//                        if(QDateTime::currentDateTime().secsTo(tTargetFile.lastModified()) < -10)
//                            break;
//                    }

                    thread()->msleep(500);
                }

                addLog("Coverter Tamamlandi: " + tFileInfo.baseName() + ".mov -> " + tFileInfo.baseName() + ".mp4");

                disconnect(mConverter, nullptr, nullptr, nullptr);
                mConverter->deleteLater();
                mConverter = nullptr;

//                tThread->quit();
//                tThread->wait();
//                tThread->exit();
//                tThread->deleteLater();
//                tThread = nullptr;

                mFiles.takeFirst();
                if(tFile.remove())
                    addLog("Dosya silindi: " + tFile.fileName());
                else
                    addLog("Hata! Dosya silinemedi: " + tFile.fileName());
            }
            else
            {
                addLog("Hata! Dosya Çevirilemez!");
            }
        }
    }
}

void MainWindow::addLog(QString pStr)
{
    ui->textEdit->setPlainText(pStr + "\n" + ui->textEdit->toPlainText());
}

void MainWindow::on_pushButton_3_clicked()
{
    QString tTemp = QFileDialog::getExistingDirectory();
    if(tTemp != "")
        ui->lineEdit_3->setText(tTemp);
}
