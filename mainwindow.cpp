#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    myReader = new rfid_Impinj();

    connect(myReader, SIGNAL(versionUpdated(QString)), ui->lbVersion, SLOT(setText(QString)));
    connect(myReader, SIGNAL(tempUpdated(QString)), ui->lbTemp, SLOT(setText(QString)));
    connect(myReader, SIGNAL(connectStatusChanged(bool)), this, SLOT(on_connectStatusChanged(bool)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnConDis_clicked()
{
    qDebug() << "btnConnect clicked";
    if(ui->btnConDis->text() == "Connect"){
        qDebug() << "Connect";
        myReader->connectReader(ui->leHost->text(), ui->lePort->text().toInt());
    } else {
        qDebug() << "Disconnect";
        myReader->disconnectReader();
    }
}

void MainWindow::on_connectStatusChanged(bool isConnected)
{
    if(isConnected){
        qDebug() << "Connected";
        ui->btnConDis->setText("Disconnect");
    } else {
        qDebug() << "Disconnected";
        ui->btnConDis->setText("Connect");
    }
}

void MainWindow::on_btnConDis_2_clicked()
{
    myReader->sendTest();
}

void MainWindow::on_btnConDis_3_clicked()
{

}
