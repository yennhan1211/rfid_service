#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QTimer>
#include <QDateTime>
#include <QHash>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QSqlQueryModel>
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

    int createDb();

signals:
    void updateTable();

private slots:
    void on_btnConDis_clicked();
    void on_connectStatusChanged(bool);

    void on_btnGetOutputPower_clicked();

    void on_btnSetOutputPower_clicked();

    void on_btnStartTimer();

    void requestTimerTimeOut();

    void checkBoxHandler(bool);

    void tagFound(epc_tag*);

    void startRequest();

    void on_btnSetDelayTime_clicked();

    void on_btnSetOffsetTime_clicked();

private:
    Ui::MainWindow *ui;
    QThread *rfid_thread;
    QTimer requestTimer;

    QHash<QString, epc_tag> tagsHolder;

    QSqlQueryModel *model;

    bool mRunning;
    QDateTime mStartTime;
    QDateTime mStartCaptureTime;
    quint64 mSessionTime;
    int timerid;

    QTimer oneShotStartTimer;
    int delayStartTime;
    int requestInterval;

    void enableUI(bool);
protected:
    void timerEvent(QTimerEvent *);
};

#endif // MAINWINDOW_H
