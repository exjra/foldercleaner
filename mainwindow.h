#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QQueue>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_2_clicked();
    void onTimerTick();
    void on_pushButton_clicked();
    void on_horizontalSlider_valueChanged(int value);
    void onDeleteTick();

    void on_pushButton_3_clicked();

    void processOutput();
    void processError();

    void processFinished(int pCode);
private:
    Ui::MainWindow *ui;
    QTimer* mTimer;
    QTimer* mDeleteTimer;
    QStringList mFiles;
    bool mDeleting;
    QProcess *mConverter;

    void addLog(QString pStr);
    QStringList getFilesUnderDir(QString pDirPath);

};

#endif // MAINWINDOW_H
