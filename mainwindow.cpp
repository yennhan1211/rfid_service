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
    mRunning = false;
    requestInterval = 1000; // default 1s
    delayStartTime = 10000; // default 10s delay

    enableUI(false);

    createDb();

    //setup tableview
    model = new QSqlQueryModel;
    model->setQuery("select * from taginfo");
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("PC"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("EPC"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("RSSI(dBm)"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("FREQ(MHz)"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("ANTENNA ID"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("TIME CAPTURED"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("DELTA TIME(ms)"));
    model->query();
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->setModel(model);
    ui->tableView->show();

    // connect signal
    oneShotStartTimer.setSingleShot(true);
    readBasicInfoTimer.setInterval(300);
    readBasicInfoTimer.setSingleShot(true);


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
    connect(myReader, SIGNAL(outputPowerUpdate(quint8)), this, SLOT(updateOutputPower(quint8)));

    connect(myReader, SIGNAL(tagFound(epc_tag*)), this, SLOT(tagFound(epc_tag*)));

    connect(&requestTimer, SIGNAL(timeout()), this, SLOT(requestTimerTimeOut()));
    connect(&oneShotStartTimer, SIGNAL(timeout()), this, SLOT(startRequest()));
    connect(&readBasicInfoTimer, SIGNAL(timeout()), this, SLOT(readBasicInfo()));
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
    db.setDatabaseName(":memory:");
    if (!db.open()) {
        QMessageBox::critical(0, qApp->tr("Cannot open database"),
            db.lastError().text(), QMessageBox::Cancel);
        return false;
    }

    // create table if not exist
    QSqlQuery query;
    query.exec("create table if not exists taginfo (id integer primary key autoincrement,"
               "pccode varchar(4), epccode varchar(256), rssi int, freq real,antid int, timecapture datetime, deltatime int)");

    // test insert
//     QString str = QString("insert into taginfo values('00 11', '11 22 33 44', 30, 895.5, '%1', 1234)").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
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
        readBasicInfoTimer.start();
//        myReader->getTemp();
        enableUI(true);
    } else {
        qDebug() << "Disconnected";
        ui->statusBar->showMessage("Disconnected");
        ui->btnConDis->setText("Connect");
        enableUI(false);
        if(mRunning){
            stopCount();
        }
    }
}

void MainWindow::on_btnGetOutputPower_clicked()
{
    myReader->getOutputPower();
//    myReader->getTemp();
//    myReader->getWorkAntenna();
//    QSqlQuery query;
//    QString str = QString("insert into taginfo(pccode,epccode,rssi,freq,antid,timecapture,deltatime) values('00 11', '11 22 33 44', 30, 895.5, '%1', 1234)").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
//    qDebug() << str;
//    query.exec(str);
//    model->setQuery("select * from taginfo");
//    model->query();
//    ui->tableView->setModel(model);
//    ui->tableView->show();
}

void MainWindow::on_btnSetOutputPower_clicked()
{
//    myReader->sendTest();
    int pw = ui->leAnt1Power->text().toInt();
    myReader->setOutputPower(pw);
    readBasicInfoTimer.start();
}

void MainWindow::on_btnStartTimer()
{
    if(!mRunning){
        startCount();
    } else {
        stopCount();
    }
}

void MainWindow::requestTimerTimeOut()
{
    qDebug() << "On requestTimerTimeOut";
    if(mRunning && myReader->getConnectStatus()){
        int sleepTimeAnt = ui->leInterval->text().toInt();
        int repeatimte = ui->leRepeat->text().toInt();
        quint8 tmp[] = {
            0xa0,0x0d,0x01,0x8a,0x00,0x01,
            0x01,0x01,0x02,0x01,0x03,0x01,0x00,0x0a//,0xb4
        };
        tmp[12] = quint8(sleepTimeAnt);
        tmp[13] = quint8(repeatimte);
    //    qDebug() << "Write test";
    //    tcpSocket->write(tmp, 5);
        myReader->sendCommand(tmp,sizeof(tmp)/sizeof(quint8));
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
        myReader->setWorkAntenna(0);
    } else if(chk == ui->chbAnt2){
        myReader->setWorkAntenna(1);
        qDebug() << "chbAnt2";
    } else if(chk == ui->chbAnt3){
        myReader->setWorkAntenna(2);
        qDebug() << "chbAnt3";
    } else if(chk == ui->chbAnt4){
        myReader->setWorkAntenna(3);
        qDebug() << "chbAnt4";
    }


}

void MainWindow::tagFound(epc_tag * tag)
{
    if(!mRunning)return;

  // check tag is exist or not
    QHash<QString, epc_tag>::const_iterator i = tagsHolder.find(tag->getKeyID());

    if (i == tagsHolder.end()) // not exist
    {
      tagsHolder.insert(tag->getKeyID(), *(tag));
      // add to db
      QString strQuery = QString("insert into taginfo(pccode,epccode,rssi,freq,antid,timecapture,deltatime)"
                            " values('%1', '%2', %3, %4, %5, '%6', %7)")
                                .arg(tag->getPCID())
                                .arg(tag->getKeyID())
                                .arg(QString::number(tag->rssiToDbm()))
                                .arg(QString::number(tag->freqToHz(), 'g', 1))
                                .arg(QString::number(tag->tagAnt->ant_id))
                                .arg(tag->tagAnt->timeCaptured.toString("yyyy-MM-dd hh:mm:ss"))
                                .arg(mStartCaptureTime.msecsTo(tag->tagAnt->timeCaptured));
      qDebug() << strQuery;
      QSqlQuery query;
      query.exec(strQuery);
      model->setQuery("select * from taginfo");
      model->query();
    } else {
      tagsHolder[tag->getKeyID()].updateAntennaInfo(*(tag));
    }


    // emit reorder table
    emit updateTable();

    if(tag != NULL){
        delete tag;
    }
}

void MainWindow::startRequest()
{
    if(mRunning){
        if(requestTimer.isActive() == false){
            int sleepInterval = ui->leInterval->text().toInt();
            int repeatimte = ui->leRepeat->text().toInt();
            if(repeatimte != 0){
                requestInterval = (sleepInterval + 30) * 4;
            } else {
                requestInterval = (sleepInterval + 30) * 4 * repeatimte;
            }
            requestTimer.start(requestInterval);
        }
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

    ui->btnSetDelayTime->setEnabled(enable);
    ui->btnSetOffsetTime->setEnabled(enable);

    ui->cbbZone->setEnabled(enable);
}

void MainWindow::startCount()
{
    mStartTime = QDateTime::currentDateTime();
    mStartCaptureTime = mStartTime;
    mRunning = true;
    ui->btnStartTime->setText("STOP");
    qDebug() <<"Start counting";
    timerid = startTimer(0);
    oneShotStartTimer.start(delayStartTime);
    if(tagsHolder.size() > 0) {
        tagsHolder.clear();
        qDebug() << tagsHolder.size();
        QSqlQuery query;
        query.exec("delete from taginfo");
        model->clear();
    }
}

void MainWindow::stopCount()
{
    killTimer(timerid);
    ui->btnStartTime->setText("START");
    mRunning = false;
    requestTimer.stop();
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

void MainWindow::on_btnSetDelayTime_clicked()
{
    bool flag = false;
    int tmp = ui->leDelayTime->text().toInt(&flag);
    if(flag){
        delayStartTime = 1000 * tmp;
    } else {
        delayStartTime = 10000; // default
        QMessageBox::critical(0, qApp->tr("Cannot open database"),
            qApp->tr("Make sure all inputs are numberic"), QMessageBox::Ok);
    }
}

void MainWindow::on_btnSetOffsetTime_clicked()
{
    QString strIn = ui->leOffsetTime->text();
    qDebug() << strIn;
    if(mRunning){
        mStartCaptureTime.addSecs(strIn.toInt());
        mStartTime.addSecs(100);
    }
}

void MainWindow::on_btnResetReader_clicked()
{
    if(myReader->getConnectStatus()){
        quint8 tmpCmd[] = {
            0xa0,0x03,0x01,0x70
        };
        myReader->sendCommand(tmpCmd, 4);
    }
}

void MainWindow::readBasicInfo()
{
    if(myReader->getConnectStatus()){
        myReader->getOutputPower();
    }
}

void MainWindow::updateOutputPower(quint8 p)
{
    QString pw = QString::number(p);
    ui->leAnt1Power->setText(pw);
    ui->leAnt2Power->setText(pw);
    ui->leAnt3Power->setText(pw);
    ui->leAnt4Power->setText(pw);
}
