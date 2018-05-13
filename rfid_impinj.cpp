#include "rfid_impinj.h"
#include <QDebug>

rfid_Impinj::rfid_Impinj(QObject *parent) :
    QObject(parent), tcpSocket(new QTcpSocket(this)), connectStatus(false)
{
    in.setDevice(tcpSocket);
    in.setVersion(QDataStream::Qt_5_3);

    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(processBuf()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(processError(QAbstractSocket::SocketError)));
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(connectionStatus()));
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(connectionStatus()));
//    connect(tcpSocket, SIGNAL())

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

void rfid_Impinj::processBuf()
{

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
