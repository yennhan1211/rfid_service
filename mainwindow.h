#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QTimer>
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

    void requestTimerTimeOut();

private:
    Ui::MainWindow *ui;
    QThread *rfid_thread;
    QTimer requestTimer;
};

#endif // MAINWINDOW_H
