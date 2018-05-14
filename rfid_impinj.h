#ifndef RFID_IMPINJ_H
#define RFID_IMPINJ_H

#include <QObject>
#include <QDataStream>
#include <QTcpSocket>
#include <QNetworkSession>

class command
{
//    Q_OBJECT
public:
    explicit command(QByteArray);

    QByteArray getData();

    quint8 calcCheckSum(QByteArray);

    QByteArray getDataToSend();

    static command buildFromBuf(QByteArray);

private:
    quint8 length;
    quint8 addr;
    quint8 cmd;
    quint8 checksum;
    QByteArray data;
};

class epc_tag : public QObject
{
    Q_OBJECT
public:
    explicit epc_tag(Qstring, int, int, int, char);

};

class rfid_Impinj : public QObject
{
    Q_OBJECT
public:
    explicit rfid_Impinj(QObject *parent = 0);
    bool connectReader(QString, quint16);
    bool disconnectReader();

    int getVersion();
    int setOutputPower(int);

    int sendCommand(command);

private:
    QTcpSocket * tcpSocket;
    QDataStream in;

    QNetworkSession *networkSession;

    bool connectStatus;
    QByteArray buffHolder;

    // Decode variable
    qint32 state;
    quint8 checkSum;
    quint8 length;
    quint8 addr;
    quint8 cmd;

signals:
    void tagFound();
    void connectStatusChanged(bool);
    void versionUpdated(QString);
    void tempUpdated(QString);
    void error(QString);
public slots:
    void processBuf();
    void processError(QAbstractSocket::SocketError socketError);
    void connectionStatus();
};

#endif // RFID_IMPINJ_H
