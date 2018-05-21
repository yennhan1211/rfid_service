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


//    rfid_thread = new QThread();

    myReader = new rfid_Impinj(parent);

//    myReader->moveToThread(rfid_thread);

    connect(ui->btnStartTime, SIGNAL(clicked()),this, SLOT(on_btnStartTimer()));
    connect(myReader, SIGNAL(versionUpdated(QString)), ui->lbVersion, SLOT(setText(QString)));
    connect(myReader, SIGNAL(tempUpdated(QString)), ui->lbTemp, SLOT(setText(QString)));
    connect(myReader, SIGNAL(connectStatusChanged(bool)), this, SLOT(on_connectStatusChanged(bool)));

    connect(myReader, SIGNAL(tagFound(epc_tag*)), this, SLOT(tagFound(epc_tag*)));

    connect(&requestTimer, SIGNAL(timeout()), this, SLOT(requestTimerTimeOut()));

    mRunning = false;


//    rfid_thread->start();

    enableUI(false);
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
        enableUI(true);
    } else {
        qDebug() << "Disconnected";
        ui->statusBar->showMessage("Disconnected");
        ui->btnConDis->setText("Connect");

        enableUI(false);
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

void MainWindow::on_btnStartTimer()
{
    if(!mRunning){
        mStartTime = QDateTime::currentDateTime();
        mRunning = true;
        ui->btnStartTime->setText("STOP");
        qDebug() <<"Start counting";
        timerid = startTimer(0);
        requestTimer.start();
    } else {
        killTimer(timerid);
        ui->btnStartTime->setText("START");
        mRunning = false;
        requestTimer.stop();
    }
}

void MainWindow::requestTimerTimeOut()
{
    qDebug() << "On requestTimerTimeOut";
    if(mRunning && myReader->getConnectStatus()){
        myReader->getTemp();
    }
}

void MainWindow::tagFound(epc_tag * tag)
{
    if(tag != NULL){
        delete tag;
    }
}

void MainWindow::enableUI(bool enable)
{
    ui->btnStartTime->setEnabled(enable);
    ui->chbAnt1->setEnabled(enable);
    ui->chbAnt2->setEnabled(enable);
    ui->chbAnt3->setEnabled(enable);
    ui->chbAnt4->setEnabled(enable);

    ui->btnGetOutputPower->setEnabled(enable);
    ui->btnSetOutputPower->setEnabled(enable);
    ui->btnSetOutputPower_2->setEnabled(enable);
    ui->btnSetOutputPower_3->setEnabled(enable);
    ui->btnSetOutputPower_4->setEnabled(enable);
}

void MainWindow::timerEvent(QTimerEvent *e)
{
    if(timerid != e->timerId()){
        timerid = e->timerId();
    }
    if(mRunning){
        mSessionTime = mStartTime.msecsTo(QDateTime::currentDateTime());
        qint64 time = mSessionTime;
        unsigned int h = time / 1000 / 60 / 60;
        unsigned int m = (time / 1000 / 60) - (h * 60);
        unsigned int s = (time / 1000) - (m * 60);
        unsigned int ms = time - (s + ((m + (h * 60))* 60)) * 1000;
        const QString diff = QString("%1:%2:%3,%4")
                                .arg(h,  2, 10, QChar('0'))
                                .arg(m,  2, 10, QChar('0'))
                                .arg(s,  2, 10, QChar('0'))
                                .arg(ms, 3, 10, QChar('0'));
        ui->lbStartTime->setText(diff);
    }
}
