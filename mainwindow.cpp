#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QStandardItemModel"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //setup status bar
    ui->statusBar->showMessage("Disconnected");

    //setup tableview
    QStandardItemModel *model = new QStandardItemModel(1,4, this);
    model->setHorizontalHeaderItem(0, new QStandardItem(QString("PC")));
    model->setHorizontalHeaderItem(1, new QStandardItem(QString("EPC")));
    model->setHorizontalHeaderItem(2, new QStandardItem(QString("RSSI")));
    model->setHorizontalHeaderItem(3, new QStandardItem(QString("FREQ")));

    ui->tableView->setModel(model);

    requestTimer.setInterval(1000);
    requestTimer.setSingleShot(false);

    myReader = new rfid_Impinj();
//    rfid_thread = new QThread();

//    myReader->moveToThread(rfid_thread);

    connect(myReader, SIGNAL(versionUpdated(QString)), ui->lbVersion, SLOT(setText(QString)));
    connect(myReader, SIGNAL(tempUpdated(QString)), ui->lbTemp, SLOT(setText(QString)));
    connect(myReader, SIGNAL(connectStatusChanged(bool)), this, SLOT(on_connectStatusChanged(bool)));

    connect(&requestTimer, SIGNAL(timeout()), this, SLOT(requestTimerTimeOut()));

//    requestTimer.start();
}

MainWindow::~MainWindow()
{
    if(requestTimer.isActive()){
        requestTimer.stop();
    }
    if(myReader->getConnectStatus()){
        myReader->disconnectReader();
    }
//    if(rfid_thread != NULL && rfid_thread->isRunning()){
//        rfid_thread->quit();
//        rfid_thread->wait();
//    }
//    delete rfid_thread;
    delete myReader;
    delete ui;
}

void MainWindow::on_btnConDis_clicked()
{
    qDebug() << "btnConnect clicked";
    if(ui->btnConDis->text() == "Connect"){
        qDebug() << "Connect";
        myReader->connectReader(ui->leHost->text(), ui->lePort->text().toInt());
        ui->statusBar->showMessage("Connecting ...");
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
        ui->statusBar->showMessage("Connected");
        myReader->getVersion();
//        myReader->getTemp();
    } else {
        qDebug() << "Disconnected";
        ui->statusBar->showMessage("Disconnected");
        ui->btnConDis->setText("Connect");
    }
}

void MainWindow::on_btnGetOutputPower_clicked()
{
    myReader->getTemp();
}

void MainWindow::on_btnSetOutputPower_clicked()
{
    myReader->sendTest();
}

void MainWindow::requestTimerTimeOut()
{
    if(myReader->getConnectStatus()){
        myReader->getTemp();
    }
}
