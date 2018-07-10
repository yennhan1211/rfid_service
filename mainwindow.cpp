#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QStandardItemModel"
#include <QSqlRelationalDelegate>
#include <QDebug>
#include <QFileDialog>
#include <QSqlResult>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    qDebug() << "Start at: " << this->thread()->currentThreadId();
    // setup ui
    ui->setupUi(this);

    mLog = new logwindow("", ui->pteLog);
    mLog->appendMsg("hello log");

    //setup status bar
    ui->statusBar->showMessage("Disconnected");

//    rfid_thread->start();
    mRunning = false;
    requestInterval = 1000; // default 1s
    delayStartTime = 1000; // default 1s delay
    operationMode = NORMALMODE; // test mode

    ui->cbbMode->setVisible(false);

    enableUI(false);

    createDb();

    mainTableViewSetHeader();

//    ui->prepareTableView->setEditTriggers(QAbstractItemView::AnyKeyPressed | QAbstractItemView::DoubleClicked);
//    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
//    ui->tableView->horizontalHeader()->resizeSections(QHeaderView::Stretch);

    // connect signal
    oneShotStartTimer.setSingleShot(true);
    readBasicInfoTimer.setInterval(300);
    readBasicInfoTimer.setSingleShot(true);

//    rfid_thread = new QThread();

    myReader = new rfid_Impinj(parent);
    myReader->setLog(mLog);

//    myReader->moveToThread(rfid_thread);

    connect(ui->btnStartTime, SIGNAL(clicked()),this, SLOT(on_btnStartTimer()));
    connect(ui->chbAnt1, SIGNAL(clicked(bool)), this, SLOT(checkBoxHandler(bool)));
    connect(ui->chbAnt2, SIGNAL(clicked(bool)), this, SLOT(checkBoxHandler(bool)));
    connect(ui->chbAnt3, SIGNAL(clicked(bool)), this, SLOT(checkBoxHandler(bool)));
    connect(ui->chbAnt4, SIGNAL(clicked(bool)), this, SLOT(checkBoxHandler(bool)));

    connect(ui->cbbMode, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeOperationMode(QString)));
    connect(ui->mainTab, SIGNAL(currentChanged(int)), this, SLOT(tabIndexChange(int)));
    connect(this, SIGNAL(updateTable(QString)), this, SLOT(onUpdateTable(QString)));

    connect(myReader, SIGNAL(versionUpdated(QString)), ui->lbVersion, SLOT(setText(QString)));
    connect(myReader, SIGNAL(tempUpdated(QString)), ui->lbTemp, SLOT(setText(QString)));
    connect(myReader, SIGNAL(connectStatusChanged(bool)), this, SLOT(on_connectStatusChanged(bool)));
    connect(myReader, SIGNAL(outputPowerUpdate(quint8)), this, SLOT(updateOutputPower(quint8)));

    connect(myReader, SIGNAL(tagFound(epc_tag*)), this, SLOT(tagFound(epc_tag*)));

    connect(&requestTimer, SIGNAL(timeout()), this, SLOT(requestTimerTimeOut()));
    connect(&oneShotStartTimer, SIGNAL(timeout()), this, SLOT(startRequest()));
    connect(&readBasicInfoTimer, SIGNAL(timeout()), this, SLOT(readBasicInfo()));

// start reader thread
    myReader->start();
}

MainWindow::~MainWindow()
{
    if(requestTimer.isActive()){
        requestTimer.stop();
    }

    if(myReader->isRunning()){
        myReader->wait(100);
    }

    if(myReader->getConnectStatus()){
        myReader->disconnectReader();
    }
    if(myReader != NULL && myReader->isRunning()){
        myReader->quit();
        myReader->wait(5000);
    }
//    delete rfid_thread;
    myReader->deleteLater();
    delete ui;
}

int MainWindow::createDb()
{

//    QString dbName = "./test" + QString::number(QDateTime::currentMSecsSinceEpoch()) + ".db";
     QSqlDatabase mCacheDb = QSqlDatabase::addDatabase("QSQLITE");
//    db.setDatabaseName(dbName);
    mCacheDb.setDatabaseName(":memory:");
    qDebug() << mCacheDb.connectionName();

    if (!mCacheDb.open()) {
        QMessageBox::critical(0, qApp->tr("Cannot open database"),
            mCacheDb.lastError().text(), QMessageBox::Cancel);
        return false;
    }

    createTable();

    return 0;
}

void MainWindow::createTable()
{
    // create table if not exist
    QSqlQuery query;

    query.exec("create table if not exists taginfo (id integer primary key autoincrement,"
               "pccode varchar(4), epccode varchar(256), rssi int, freq real,antid int, timecapture datetime, deltatime int)");
    qDebug() << "->>" << query.lastError().text();
    query.exec("create table if not exists racer (id integer primary key autoincrement,startnumber int, tagid varchar(256), surname varchar(128),"
               "uciid int, category varchar(128), nationality varchar(128), club varchar(128))");
    query.exec("create table if not exists record (id integer primary key autoincrement,startnumber int, surname varchar(128), tagid varchar(256),"
               "deltatime int)");
    qDebug() << "->>" << query.lastError().text();
    qDebug() << QSqlDatabase::database().tables();
}

void MainWindow::mainTableViewSetHeader()
{
//    if(model != NULL) delete model;
//    if(racerModel != NULL) delete racerModel;
    model = new QSqlTableModel;
    racerModel = new QSqlTableModel;

    racerModel->setTable("racer");
    racerModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    racerModel->select();
    model->setTable("record");
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Start Number"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Sur Name"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Tag ID"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Delta Time"));
    model->select();

    ui->tableView->setModel(model);
    ui->prepareTableView->setModel(racerModel);

    ui->prepareTableView->setEditTriggers(QAbstractItemView::AnyKeyPressed | QAbstractItemView::DoubleClicked);
//    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWindow::collectTableViewSetHeader()
{
    modelPrepare->setQuery("select * from racer");
    modelPrepare->setHeaderData(0, Qt::Horizontal, QObject::tr("Start number"));
    modelPrepare->setHeaderData(1, Qt::Horizontal, QObject::tr("Tag ID"));
    modelPrepare->setHeaderData(2, Qt::Horizontal, QObject::tr("Sur Name"));
    modelPrepare->setHeaderData(3, Qt::Horizontal, QObject::tr("Uci ID"));
    modelPrepare->setHeaderData(4, Qt::Horizontal, QObject::tr("Category"));
    modelPrepare->setHeaderData(5, Qt::Horizontal, QObject::tr("Nationality"));
    modelPrepare->setHeaderData(6, Qt::Horizontal, QObject::tr("Club"));
    modelPrepare->query();
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
            0x01,0x00,0x02,0x00,0x03,0x00,0x00,0x0a
//            0x01,0x01,0x02,0x01,0x03,0x01,0x00,0x0a
        };
        tmp[12] = quint8(sleepTimeAnt);
        tmp[13] = quint8(repeatimte);
        int ret = myReader->sendCommand(tmp,sizeof(tmp)/sizeof(quint8));
        if(ret == -1) {
            qDebug() << "Bad command " << ret;
            stopCount();
        }
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
    qDebug () << "--->" << operationMode;
    QString strQuery = "";
//    if(!mRunning)return;

  // check tag is exist or not
    if(operationMode == NORMALMODE){
        QHash<QString, epc_tag>::const_iterator i = tagsHolder.find(tag->getKeyID());

        if (i == tagsHolder.end()) // not exist
        {
          tagsHolder.insert(tag->getKeyID(), *(tag));
          // add to db
          strQuery = QString("insert into taginfo(pccode,epccode,rssi,freq,antid,timecapture,deltatime)"
                                " values('%1', '%2', %3, %4, %5, '%6', %7)")
                                    .arg(tag->getPCID())
                                    .arg(tag->getKeyID())
                                    .arg(QString::number(tag->rssiToDbm()))
                                    .arg(QString::number(tag->freqToHz(), 'g', 1))
                                    .arg(QString::number(tag->tagAnt->ant_id))
                                    .arg(tag->tagAnt->timeCaptured.toString("yyyy-MM-dd hh:mm:ss"))
                                    .arg(mStartCaptureTime.msecsTo(tag->tagAnt->timeCaptured));

        } else {
          tagsHolder[tag->getKeyID()].updateAntennaInfo(*(tag));
        }
    } else if(operationMode == TESTMODE){
        strQuery = QString("insert into taginfo(pccode,epccode,rssi,freq,antid,timecapture,deltatime)"
                              " values('%1', '%2', %3, %4, %5, '%6', %7)")
                                  .arg(tag->getPCID())
                                  .arg(tag->getKeyID())
                                  .arg(QString::number(tag->rssiToDbm()))
                                  .arg(QString::number(tag->freqToHz(), 'g', 1))
                                  .arg(QString::number(tag->tagAnt->ant_id))
                                  .arg(tag->tagAnt->timeCaptured.toString("yyyy-MM-dd hh:mm:ss"))
                                  .arg(mStartCaptureTime.msecsTo(tag->tagAnt->timeCaptured));
    } else if(operationMode == COLLECTTAGMODE){
        QHash<QString, epc_tag>::const_iterator i = tagsHolder.find(tag->getKeyID());
        qDebug() << ">>" << tagsHolder.size() << tag->getKeyID();
        if (i == tagsHolder.end()) // not exist
        {
            tagsHolder.insert(tag->getKeyID(), *(tag));
            {
                QSqlQuery query;
                QString selectStr = QString("select surname from racer where tagid = '%1'").arg(tag->getKeyID());
                query.exec(selectStr);
                qDebug() << query.lastError().text();
                if(!query.next()){
                    strQuery = QString("insert into racer (tagid) values ('%1')").arg(tag->getKeyID());
                }
            }

        }
    }
    if(strQuery != "") {

        QSqlQuery query;
        query.exec(strQuery);
        qDebug() << "query " << strQuery << query.lastError().text();
        if(operationMode == COLLECTTAGMODE)
            racerModel->select();
        // emit reorder table
        if(operationMode == NORMALMODE)
            emit updateTable(tag->getKeyID());
    }
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
            qDebug() << "requestInterval" << requestInterval;
            requestTimer.start(requestInterval);
        }
//        myReader->setReadFastSwitching(true);
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

    myReader->clearErrorFlag();

    {
        QSqlQuery query;
        query.exec("delete from taginfo");
        query.exec("delete from record");
//        createTable();
        model->select();
    }

    if(tagsHolder.size() > 0) {
        tagsHolder.clear();
    }
}

void MainWindow::stopCount()
{
    qDebug() << "stop counting";
    killTimer(timerid);
    ui->btnStartTime->setText("START");
    mRunning = false;
    requestTimer.stop();
//    myReader->setReadFastSwitching(false);
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

void MainWindow::changeOperationMode(QString str)
{
    qDebug() << "Mode changeged " << str;
    if(str == "Test"){
        operationMode = MainWindow::TESTMODE;
    } else if(str == "Normal") {
        operationMode = MainWindow::NORMALMODE;
    }
}

void MainWindow::tabIndexChange(int i)
{
    if(i == 0){
        QString str = ui->cbbMode->currentText();
        if(str == "Test"){
            operationMode = MainWindow::TESTMODE;
        } else if(str == "Normal") {
            operationMode = MainWindow::NORMALMODE;
        }
    } else if (i == 1){
        operationMode = COLLECTTAGMODE;
    }
}

void MainWindow::on_btnCollectTag_clicked()
{
    qDebug() << "On on_btnCollectTag_clicked";
    if(myReader->getConnectStatus()){
        myReader->clearErrorFlag();
        quint8 tmp[] = {
            0xa0,0x0d,0x01,0x8a,0x00,0x01,
//            0x01,0x01,0x02,0x01,0x03,0x01,0x00,0x0a
            0x01,0x00,0x02,0x00,0x03,0x00,0x00,0x0a
        };
        tmp[12] = quint8(5);
        tmp[13] = quint8(10);
        qDebug() << myReader->sendCommand(tmp,sizeof(tmp)/sizeof(quint8));
    }
}

void MainWindow::on_btnSave2File_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save data"), "",
            tr("MTB data file (*.db);"));

    if (fileName.isEmpty())
           return;
    else {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
               QMessageBox::information(this, tr("Unable to open file"),
                   file.errorString());
               return;
        }
        qDebug() << fileName;

        QSqlDatabase::removeDatabase(QSqlDatabase::database().connectionName());
        QSqlDatabase mCacheDb = QSqlDatabase::addDatabase("QSQLITE");
        mCacheDb.setDatabaseName(fileName);
        qDebug() << mCacheDb.open() << mCacheDb.isOpen();
        createTable();
        mainTableViewSetHeader();
        tagsHolder.clear();
    }
}

void MainWindow::on_btnResetReader_7_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Load data"), "",
            tr("MTB data file (*.db);"));
    if (fileName.isEmpty())
           return;
    else {
        qDebug() << fileName;
        QSqlDatabase::removeDatabase(QSqlDatabase::database().connectionName());
        QSqlDatabase mCacheDb = QSqlDatabase::addDatabase("QSQLITE");
        mCacheDb.setDatabaseName(fileName);
        qDebug() << mCacheDb.open() << mCacheDb.isOpen();
        createTable();
        mainTableViewSetHeader();
        tagsHolder.clear();
    }
}

void MainWindow::onUpdateTable(QString tagId)
{
    QSqlQuery query;
    QString tagInfoStr = QString("select deltatime from taginfo where epccode = '%1'").arg(tagId);
    QString reacerInfo = QString("select startnumber,surname from racer where tagid = '%1'").arg(tagId);
    query.exec(tagInfoStr);
    qDebug() << query.lastError().text();
    int delta = 0;
    int startnumber = 0;
    QString name = "";
    if(query.next()){
        delta = query.value("deltatime").toInt();
    }
    query.exec(reacerInfo);
    qDebug() << query.lastError().text();
    if(query.next()){
        startnumber = query.value("startnumber").toInt();
        name = query.value("surname").toString();
    }
    QString strQuery = QString("insert into record (startnumber, surname, tagid, deltatime)"
                               " values (%1,'%2','%3', %4)")
                                .arg(startnumber).arg(name).arg(tagId).arg(delta);
    query.exec(strQuery);
    qDebug() << query.lastError().text();
    model->select();
}

