#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QStandardItemModel"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // setup ui
    ui->setupUi(this);

    //setup status bar
    ui->statusBar->showMessage("Disconnected");

//    rfid_thread->start();

    enableUI(false);

    createDb();
    //setup tableview


//    ui->tableView->setColumnWidth(0, (int)(ui->tableView->width()/5));
//    ui->tableView->setColumnWidth(1, (int)(ui->tableView->width()/5));
//    ui->tableView->setColumnWidth(2, (int)(ui->tableView->width()/5));
//    ui->tableView->setColumnWidth(3, (int)(ui->tableView->width()/5));
//    ui->tableView->setColumnWidth(4, (int)(ui->tableView->width()/5));
    model = new QSqlQueryModel;
    model->setQuery("select * from taginfo");
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("PC"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("EPC"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("RSSI"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("FREQ"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("TIME CAPTURED"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("DELTA TIME"));
    model->query();
    ui->tableView->setModel(model);
    ui->tableView->show();

    // connect signal
    requestTimer.setInterval(1000);
    requestTimer.setSingleShot(false);


//    rfid_thread = new QThread();

    myReader = new rfid_Impinj(parent);

//    myReader->moveToThread(rfid_thread);

    connect(ui->btnStartTime, SIGNAL(clicked()),this, SLOT(on_btnStartTimer()));
    connect(ui->chbAnt1, SIGNAL(clicked(bool)), this, SLOT(checkBoxHandler(bool)));
    connect(ui->chbAnt2, SIGNAL(clicked(bool)), this, SLOT(checkBoxHandler(bool)));
    connect(ui->chbAnt3, SIGNAL(clicked(bool)), this, SLOT(checkBoxHandler(bool)));
    connect(ui->chbAnt4, SIGNAL(clicked(bool)), this, SLOT(checkBoxHandler(bool)));

    connect(myReader, SIGNAL(versionUpdated(QString)), ui->lbVersion, SLOT(setText(QString)));
    connect(myReader, SIGNAL(tempUpdated(QString)), ui->lbTemp, SLOT(setText(QString)));
    connect(myReader, SIGNAL(connectStatusChanged(bool)), this, SLOT(on_connectStatusChanged(bool)));

    connect(myReader, SIGNAL(tagFound(epc_tag*)), this, SLOT(tagFound(epc_tag*)));

    connect(&requestTimer, SIGNAL(timeout()), this, SLOT(requestTimerTimeOut()));

    mRunning = false;
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

int MainWindow::createDb()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("./mydb.db");
    if (!db.open()) {
        QMessageBox::critical(0, qApp->tr("Cannot open database"),
            qApp->tr("Unable to establish a database connection.\n"
                     "This example needs SQLite support. Please read "
                     "the Qt SQL driver documentation for information how "
                     "to build it.\n\n"
                     "Click Cancel to exit."), QMessageBox::Cancel);
        return false;
    }

    // create table if not exist
    QSqlQuery query;
    query.exec("create table if not exists taginfo (id int primary key, "
               "pccode varchar(4), epccode varchar(128), rssi int, freq real, timecapture datetime, deltatime int)");

    // test insert
    // QString str = QString("insert into taginfo values(0, '00 11', '11 22 33 44', 30, 895.5, '%1', 1234)").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
//    qDebug() << str;
//    query.exec(str);

    return 0;
}

void MainWindow::on_btnConDis_clicked()
{
    qDebug() << "btnConnect clicked";
    if(ui->btnConDis->text() == "Connect"){
        qDebug() << "Connect";
        myReader->connectReader(ui->leHost->text(), ui->lePort->text().toInt());
        ui->statusBar->showMessage("Connecting ...");
        ui->btnConDis->setEnabled(false);
    } else {
        qDebug() << "Disconnect";
        myReader->disconnectReader();
    }
}

void MainWindow::on_connectStatusChanged(bool isConnected)
{
    ui->btnConDis->setEnabled(true);
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
        mStartCaptureTime = QDateTime::currentDateTime();
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

void MainWindow::checkBoxHandler(bool isChecked)
{
    qDebug() << "Checked" << isChecked;
    QCheckBox *chk = (QCheckBox*)sender();
    if(isChecked){
        chk->setText("On");
    } else {
        chk->setText("Off");
    }
    if(chk == ui->chbAnt1){
        qDebug() << "chbAnt1";
    } else if(chk == ui->chbAnt2){
        qDebug() << "chbAnt2";
    } else if(chk == ui->chbAnt3){
        qDebug() << "chbAnt3";
    } else if(chk == ui->chbAnt4){
        qDebug() << "chbAnt4";
    }

}

void MainWindow::tagFound(epc_tag * tag)
{
  // check tag is exist or not
    QHash<QString, epc_tag>::const_iterator i = tagsHolder.find(tag->getKeyID());

    if (i == tagsHolder.end()) // not exist
    {
      tagsHolder.insert(tag->getKeyID(), *(tag));
    } else {
      tagsHolder[tag->getKeyID()].updateAntennaInfo(*(tag));
    }

    // emit reorder table

    if(tag != NULL){
        delete tag;
    }
}

void MainWindow::enableUI(bool enable)
{
    ui->btnStartTime->setEnabled(enable);
//    ui->chbAnt1->setEnabled(enable);
//    ui->chbAnt2->setEnabled(enable);
//    ui->chbAnt3->setEnabled(enable);
//    ui->chbAnt4->setEnabled(enable);

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
