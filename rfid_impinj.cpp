#include "rfid_impinj.h"
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

    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(processBuf()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(processError(QAbstractSocket::SocketError)));
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(connectionStatus()));
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(connectionStatus()));
    connect(this, SIGNAL(cmdDataArrival(quint8,quint8,quint8,quint8*)), this, SLOT(processDataArrival(quint8,quint8,quint8,quint8*)));
//    connect(tcpSocket, SIGNAL())


//    sendTest();
}

bool rfid_Impinj::connectReader(QString host, quint16 port)
{
    qDebug() << "connectToHost";
    tcpSocket->abort();
    tcpSocket->connectToHost(host, port);
}

bool rfid_Impinj::disconnectReader()
{
    qDebug() << "disconnectFromHost";
    tcpSocket->close();
    tcpSocket->disconnectFromHost();
    tcpSocket->abort();
}

int rfid_Impinj::sendCommand(command cmd)
{
    if(connectStatus){
        tcpSocket->write(cmd.getDataToSend());
        return 0;
    }
    return -1;
}

void rfid_Impinj::sendTest()
{
    char tmp[5] = {
        0xa0,0x03,0x01,0x72,0xea
    };
    qDebug() << "Write test";
    tcpSocket->write(tmp, 5);
}

void rfid_Impinj::processDataArrival(quint8 addr, quint8 cmd, quint8 len, quint8 * data){
    qDebug() << "Addr: " << addr;
    qDebug() << "Cmd: " << cmd;
    qDebug() << "Length: " << len;
    if(data != NULL) {
        delete data;
    }
}

void rfid_Impinj::processBuf()
{
    qint32 bytes = tcpSocket->bytesAvailable();

    char * buffRead = new char [bytes];

    if (buffRead == NULL) {
        qDebug() << "Cannot allocate mem";
        return;
    }

    tcpSocket->read(buffRead, bytes);

    buffHolder.append(buffRead, bytes);

    int i = 0;

    for(i = 0; i < buffHolder.length(); i++){
        switch(state){
        case 0:
            if(buffHolder[i] == (quint8)160) {
                checkSum += buffHolder[i];
                state++; // header 160 (A0)
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


epc_tag::epc_tag(QString, int, int, int, char)
{

}
