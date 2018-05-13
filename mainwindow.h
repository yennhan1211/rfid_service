#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
