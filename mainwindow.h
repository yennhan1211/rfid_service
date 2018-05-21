#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QTimer>
#include <QDateTime>
#include <rfid_impinj.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    rfid_Impinj *myReader;

private slots:
    void on_btnConDis_clicked();
    void on_connectStatusChanged(bool);

    void on_btnGetOutputPower_clicked();

    void on_btnSetOutputPower_clicked();

    void on_btnStartTimer();

    void requestTimerTimeOut();

    void tagFound(epc_tag*);

private:
    Ui::MainWindow *ui;
    QThread *rfid_thread;
    QTimer requestTimer;

    bool mRunning;
    QDateTime mStartTime;
    quint64 mSessionTime;
    int timerid;

    void enableUI(bool);
protected:
    void timerEvent(QTimerEvent *);
};

#endif // MAINWINDOW_H
