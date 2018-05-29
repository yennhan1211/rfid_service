#include "rfid_impinj.h"
#include "rfid_impinj_cmd.h"
#include <QDebug>

rfid_Impinj::rfid_Impinj(QObject *parent) :
    QObject(parent), tcpSocket(new QTcpSocket(this)), connectStatus(false)
{
    in.setDevice(tcpSocket);
    in.setVersion(QDataStream::Qt_5_3);

    state = 0;
    checkSum = 0;
    length = 0;
    addr = 0;
    cmd = 0;
    lengthCount = 0;

    //
    repeat = 0;
    interval = 0;

    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(processBuf()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(processError(QAbstractSocket::SocketError)));
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(connectionStatus()));
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(connectionStatus()));
    connect(this, SIGNAL(cmdDataArrival(quint8,quint8,quint8,quint8*)), this, SLOT(processDataArrival(quint8,quint8,quint8,quint8*)));
//    connect(tcpSocket, SIGNAL())

//    proxy.setType(QNetworkProxy::HttpProxy);
//    proxy.setHostName("fsoft-proxy");
//    proxy.setPort(8080);
//    proxy.setUser("anhnn28");
//    proxy.setPassword("Congtubot!2");
//    tcpSocket->setProxy(proxy);
//    QNetworkProxy::setApplicationProxy(proxy);
//    tcpSocket->setProxy(&proxy);
//    sendTest();
}

bool rfid_Impinj::connectReader(QString host, quint16 port)
{
    qDebug() << "connectToHost";
    tcpSocket->abort();
    tcpSocket->connectToHost(host, port);

    return true;
}

bool rfid_Impinj::disconnectReader()
{
    qDebug() << "disconnectFromHost";
    tcpSocket->close();
    tcpSocket->disconnectFromHost();
    tcpSocket->abort();

    return true;
}

int rfid_Impinj::getVersion()
{
    qDebug() << "getVersion";
    quint8 cmdTmp[4] = {
        0xa0,0x03,0x01,0x72
    };

    return sendCommand(cmdTmp, 4);
}

int rfid_Impinj::getTemp()
{
    qDebug() << "getTemp";
    quint8 cmdTmp[4] = {
        0xa0,0x03,0x01,0x7b
    };

    return sendCommand(cmdTmp, 4);
}

int rfid_Impinj::getRegionFreq()
{
    qDebug() << "getRegionFreq";
    quint8 cmdTmp[4] = {
        0xa0,0x03,0x01,0x79
    };

    return sendCommand(cmdTmp, 4);
}

int rfid_Impinj::setOutputPower(int power)
{
    qDebug() << "setOutputPower for all";
    quint8 cmdTmp[5] = {
        0xa0,0x04,0x01,0x76
    };

    cmdTmp[4] = (quint8)power;

    return sendCommand(cmdTmp, 5);
}

int rfid_Impinj::setOutputPower(int power0, int power1, int power2, int power3)
{
//    qDebug() << "setOutputPower for " << ant_id;
    quint8 cmdTmp[8] = {
        0xa0,0x07,0x01,0x76
    };

    cmdTmp[4] = (quint8)power0 & 0x21;
    cmdTmp[5] = (quint8)power1 & 0x21;
    cmdTmp[6] = (quint8)power2 & 0x21;
    cmdTmp[7] = (quint8)power3 & 0x21;

    return sendCommand(cmdTmp, 8);
}

int rfid_Impinj::getOutputPower()
{
    quint8 cmdTmp[4] = {
        0xa0,0x07,0x01,0x77
    };
    return sendCommand(cmdTmp, 8);
}

int rfid_Impinj::getWorkAntenna()
{
    qDebug() << "getWorkAntenna";
    quint8 cmdTmp[4] = {
        0xa0,0x03,0x01,0x75
    };

    return sendCommand(cmdTmp, 4);
}

int rfid_Impinj::setWorkAntenna(int ant_id)
{
    qDebug() << "setWorkAntenna";
    quint8 cmdTmp[5] = {
        0xa0,0x04,0x01,0x74
    };

    cmdTmp[4] = (quint8)ant_id & 0x03;

    return sendCommand(cmdTmp, 5);
}

void rfid_Impinj::setIntervalSwitchAnt(int i)
{
    interval = i;
}

int rfid_Impinj::getIntervalSwitchAnt()
{
    return interval;
}

int rfid_Impinj::sendCommand(command cmd)
{
    if(connectStatus){
        tcpSocket->write(cmd.getDataToSend());
        return 0;
    }
    return -1;
}

int rfid_Impinj::sendCommand(quint8 *arr, int len)
{
    quint8 cks = 0;

    for(int i = 0; i < len; i++){
        cks += arr[i];
    }

    cks = (~cks) + 1;

    // for debug


    if(connectStatus){
        tcpSocket->write((char*)arr, len);
        tcpSocket->write((char*)&cks);
        return 0;
    }

    return -1;
}

void rfid_Impinj::sendTest()
{
//    quint8 tmp[4] = {
//        0xa0,0x03,0x01,0x72
//    };
//
    quint8 tmp[] = {
        0xa0,0x0d,0x01,0x8a,0x00,0x01,
        0x01,0x01,0x02,0x01,0x03,0x01,0x00,0x0a//,0xb4
    };
//    qDebug() << "Write test";
//    tcpSocket->write(tmp, 5);
    sendCommand(tmp,sizeof(tmp)/sizeof(quint8));
}

QString rfid_Impinj::rssiToString(quint8 rssi)
{
    return QString::number(rssiToDbm(rssi)) + "dBm";
}

QString rfid_Impinj::freqToString(quint8 freq)
{
    return QString::number(freqToHz(freq), 'g', 1);
}

int rfid_Impinj::rssiToDbm(quint8 rssi)
{
    int retVal = 0;

    if(rssi >= (quint8)90) {
        retVal = rssi - (quint8)98 + (-31);
    } else if(rssi >= (quint8)66) {
        retVal = rssi - (quint8)89 + (-41);
    } else if (rssi == (quint8)65){
        retVal = -55;
    } else if(rssi >= (quint8)31){
        retVal = rssi - (quint8)64 + (-66);
    }

    return retVal;
}

float rfid_Impinj::freqToHz(quint8 freq)
{
    float retVal = 0;
    if(freq <= (quint8)6){
        retVal = 865 + 0.5 * (float)freq;
    } else {
        retVal = 902 + (float)(freq - 7) * 0.5;
    }

    return retVal;
}

bool rfid_Impinj::getConnectStatus()
{
    return connectStatus;
}

void rfid_Impinj::processDataArrival(quint8 addr, quint8 cmd, quint8 len, quint8 * data){
    qDebug() << "Addr: " << addr;
    qDebug() << "Cmd: " << cmd;
    qDebug() << "Length: " << len;
    QString version;
    QString temp;
    switch(cmd) {
    case CMD_GET_TEMP:
        if(data[0] == (quint8)0x01) {
            temp = "Temp: -" + QString::number(data[1]) + " C";
        } else {
            temp = "Temp: " + QString::number(data[1]) + " C";
        }
        emit tempUpdated(temp);
        break;
    case CMD_GET_VERSION:
        version = "Ver: " + QString::number(data[0]) + "." + QString::number(data[1]);
        emit versionUpdated(version);
        break;
    case CMD_GET_WORK_ANTENNA:
        qDebug() << "Working ant: " << data[0];
        break;
    case CMD_SET_WORK_ANTENNA:
        break;
    case CMD_REAL_TIME_INVENTORY:
        if(len == 4) {
            // emit error
        } else if(len == 12) {
            // emit total read
        } else if(len == 21) {
            // emit tag found
        }
        break;
    case CMD_FAST_SWITCH_ANT_INVENTORY:
        if(len == 7){
            qDebug() << "End of fast switch ant inventory";
        } else if(len == 2){
            qDebug() << "Ant " << QString::number(data[0]) << " is missing";
        } else {
            epc_tag *tag = new epc_tag(data, len);
            qDebug() << "Tag found " << tag->toString();
            emit tagFound(tag);
        }
        break;
    case CMD_RESET:
        qDebug() << "Reset reader failed";
        emit resetReader(false);
        break;
    case CMD_GET_OUTPUT_POWER:
        qDebug() << "get output power";
        emit outputPowerUpdate(data[0]);
        break;
    case CMD_SET_OUTPUT_POWER:
        qDebug() << "set output power";
//        emit setOutputPowerResult();
        break;
    default:
        qDebug() << "Unknown command " << cmd;
        break;
    }

    if(data != NULL) {
        delete data;
    }
}

void rfid_Impinj::processBuf()
{
    qint32 bytes = tcpSocket->bytesAvailable();

    qDebug() << "bytesAvailable: " << bytes;

    char * buffRead = new char [bytes];

    if (buffRead == NULL) {
        qDebug() << "Cannot allocate mem";
        return;
    }

    tcpSocket->read(buffRead, bytes);

    buffHolder.append(buffRead, bytes);

//    qDebug() << "buffHolder before: " << buffHolder;

    int i = 0;

    for(i = 0; i < buffHolder.length(); i++){
        switch(state){
        case 0:
            if((quint8)buffHolder[i] == (quint8)160) {
                checkSum += buffHolder[i];
                state++; // header 160 (A0)
                qDebug() << "Found header";
            }
            break;
        case 1:
            length = buffHolder[i]; state++;
            checkSum += length;
            // create buffer to hold data
            cmdData = new quint8[length - 3];
            break;
        case 2:
            addr = buffHolder[i]; state++;
            checkSum += addr;
            lengthCount++;
            break;
        case 3:
            cmd = buffHolder[i];
            lengthCount++;
            checkSum += cmd;
            state++;
            break;
        case 4:
                cmdData[lengthCount - 2] = buffHolder[i];
                checkSum += buffHolder[i];
                lengthCount++;
                if(lengthCount == (length -1)) {
                    state++; // end of data
                }
            break;
        case 5:
            // calc checksum
            checkSum = (~checkSum) + 1;
            if((quint8)checkSum == (quint8)buffHolder[i]) {
                // emit cmd data is found
                qDebug() << "Command data is found";
                quint8 *tmpData = new quint8[length - 3];
                memcpy(tmpData, cmdData, length -3);
                // addr cmd chks
                emit cmdDataArrival(addr, cmd, length -3, tmpData);
            } else {
                qDebug() << "Wrong checksum " << checkSum << "-" << (quint8)buffHolder[i];
            }
            // reset state
            state = 0;
            checkSum = 0;
            lengthCount = 0;
            length = 0;
            addr = 0;
            cmd = 0;
            if (cmdData != NULL) {
                delete cmdData;
                cmdData = NULL;
            }
            break;
        default:
            qDebug() << "Shouldn't be here";
            break;
        }
    }

    buffHolder.remove(0, i);

//    qDebug() << "buffHolder after: " << buffHolder;
}

void rfid_Impinj::processError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
        case QAbstractSocket::RemoteHostClosedError:
            break;
        case QAbstractSocket::HostNotFoundError:
            qDebug() << "Host not found";
            break;
        case QAbstractSocket::ConnectionRefusedError:
            qDebug() << "Connection refuse";
            break;
        default:
            qDebug() << tcpSocket->errorString();
        }
    connectStatus = false;
    emit connectStatusChanged(connectStatus);
}

void rfid_Impinj::connectionStatus()
{
//    qDebug() << "Connected";
    connectStatus = !connectStatus;
    emit connectStatusChanged(connectStatus);
}


command::command(QByteArray)
{

}

QByteArray command::getData()
{

}

quint8 command::calcCheckSum(QByteArray)
{

}

QByteArray command::getDataToSend()
{

}

command command::buildFromBuf(QByteArray)
{

}

epc_tag::epc_tag(quint8 *buff, quint8 len)
{
    freq = quint8(buff[0] >> 2);
    ant_id = quint8(buff[0] & 0x03);

    pc[0] = buff[1];
    pc[1] = buff[2];

    pcID.sprintf("%x %x", pc[0], pc[1]);

    epc_len = len - 4;

    epc = new quint8[len - 4];

    QString tmpStr;
    for (int i = 0; i < (len -4); ++i) {
        epc[i] = buff[i + 3];
        tmpStr.sprintf("%x", epc[i]);
        keyID.append(tmpStr);
    }

    rssi = buff[len -1];
    tagAnt = new antenna(ant_id, rssi, freq);
    antHolder.append(*tagAnt);
}

epc_tag::epc_tag(const epc_tag &tag)
{
    qDebug() << "epc_tag copy constuctor" ;
    freq = tag.freq;
    ant_id = tag.ant_id;

    pc[0] = tag.pc[0];
    pc[1] = tag.pc[1];

    epc = new quint8[tag.epc_len];

    for (int i = 0; i < (tag.epc_len); ++i) {
        epc[i] = tag.epc[i + 3];
    }

    epc_len = tag.epc_len;
    keyID = tag.keyID;
    pcID = tag.pcID;

    tagAnt = new antenna(0,0,0);
    *tagAnt = *(tag.tagAnt);

    rssi = tag.rssi;
    startTimePerTag = tag.startTimePerTag;
    antHolder.clear();
    for(int i =0; i < tag.antHolder.length(); i++) {
        antHolder.append(tag.antHolder[i]);
    }
}

epc_tag::~epc_tag()
{
    delete epc;
}

void epc_tag:: updateAntennaInfo(epc_tag& tag){
    qDebug() << "updateAntennaInfo " << tag.tagAnt->ant_id;
    antHolder.append(*(tag.tagAnt));
}

int epc_tag::rssiToDbm()
{
    int retVal = 0;

    if(rssi >= (quint8)90) {
        retVal = rssi - (quint8)98 + (-31);
    } else if(rssi >= (quint8)66) {
        retVal = rssi - (quint8)89 + (-41);
    } else if (rssi == (quint8)65){
        retVal = -55;
    } else if(rssi >= (quint8)31){
        retVal = rssi - (quint8)64 + (-66);
    }

    return retVal;
}

float epc_tag::freqToHz()
{
    float retVal = 0;
    if(freq <= (quint8)6){
        retVal = 865 + 0.5 * (float)freq;
    } else {
        retVal = 902 + (float)(freq - 7) * 0.5;
    }

    return retVal;
}

epc_tag &epc_tag::operator=(const epc_tag &tag)
{
    qDebug() << "epc_tag overload asignmment operator" ;
    freq = tag.freq;
    ant_id = tag.ant_id;

    pc[0] = tag.pc[0];
    pc[1] = tag.pc[1];

    epc = new quint8[tag.epc_len];
    epc_len = tag.epc_len;

    for (int i = 0; i < (tag.epc_len); ++i) {
        epc[i] = tag.epc[i + 3];
    }

    epc_len = tag.epc_len;
    keyID = tag.keyID;
    pcID = tag.pcID;

    if(tagAnt != NULL)delete tagAnt;

    tagAnt = new antenna(0,0,0);
    *tagAnt = *(tag.tagAnt);

    rssi = tag.rssi;
    startTimePerTag = tag.startTimePerTag;
    antHolder.clear();
    for(int i =0; i < tag.antHolder.length(); i++) {
        antHolder.append(tag.antHolder[i]);
    }

    return *this;
}

QString epc_tag::toString()
{
    QString str, ret;
    for(int i = 0; i < 2; i++) {
        ret.append(str.sprintf(" 0x%x", pc[i]));
    }
    for(int i = 0; i < epc_len; i++) {
        ret.append(str.sprintf(" 0x%x", epc[i]));
    }

    return ret;
}

QString epc_tag::getKeyID()
{
    return keyID;
}

QString epc_tag::getPCID()
{
    return pcID;
}


antenna::antenna(quint8 antId, quint8 rssi, quint8 frqAnt):ant_id(antId),rssi(rssi),freq(frqAnt)
{
    timeCaptured = QDateTime::currentDateTime();
}

antenna::antenna(const antenna &ant)
{
    rssi = ant.rssi;
    freq = ant.freq;
    ant_id = ant.ant_id;

    timeCaptured= ant.timeCaptured;
}

antenna &antenna::operator=(const antenna &ant)
{
    rssi = ant.rssi;
    freq = ant.freq;
    ant_id = ant.ant_id;

    timeCaptured= ant.timeCaptured;
}
