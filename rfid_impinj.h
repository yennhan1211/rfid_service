#ifndef RFID_IMPINJ_H
#define RFID_IMPINJ_H

#include <QObject>
#include <QDataStream>
#include <QTcpSocket>
#include <QNetworkSession>
#include <QNetworkProxy>
#include <QDateTime>
#include <QTimer>
#include <QThread>
#include <logwindow.h>

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
    quint8 lengthCount;
    quint8 addr;
    quint8 cmd;
    quint8 checksum;
    quint8 * cmdData;
    QByteArray data;
};

class antenna {
public:
    explicit antenna(quint8 antId, quint8 rssi,quint8 frqAnt);
    antenna(const antenna &ant);
    antenna& operator= (const antenna &ant);
    quint8 rssi;
    quint8 freq;
    quint8 ant_id;
    QDateTime timeCaptured;
};

class epc_tag : public QObject
{
    Q_OBJECT
public:
    explicit epc_tag(quint8 *buff = 0, quint8 len = 0);
    epc_tag(const epc_tag &tag);
    ~epc_tag();
    QString toString();
    quint8 getRSSI();
    quint8 getFreq();
    QString getKeyID();
    QString getPCID();
    void updateAntennaInfo(epc_tag&);

    QDateTime finalTimeCaptured;

    int rssiToDbm();
    float freqToHz();

    bool friend operator ==(const epc_tag &tag1, const epc_tag &tag2) {
        for(int i = 0; i < 2; i++) {
            if(tag1.pc[i] != tag2.pc[i]){
                return false;
            }
        }

        if(tag1.epc_len != tag2.epc_len){
            return false;
        }

        for(int i = 0; i < tag1.epc_len; i++) {
            if(tag1.epc[i] != tag2.epc[i]){
                return false;
            }
        }
        return true;
    }
    epc_tag& operator= (const epc_tag &tag);
    void updateStartTimePerTag(QDateTime& stime);
    antenna *tagAnt;
private:
    quint8 readCount;
    quint8 pc[2];
    quint8 rssi;
    quint8 freq;
    quint8 ant_id;
    quint8 * epc;
    quint8 epc_len;


    QDateTime startTimePerTag;
    QDateTime captureTime;

    QString keyID;
    QString pcID;


    QList<antenna> antHolder;
//    bool friend operator !=(const epc_tag &tag1, const epc_tag &tag2);
//    friend std::ostream& operator << (std::ostream &out, const epc_tag &tag);
};

//bool epc_tag::operator ==(const epc_tag &tag2)
//{
//    for(int i = 0; i < 2; i++) {
//        if(this->pc[i] != tag2.pc[i]){
//            return false;
//        }
//    }
//    for(int i = 0; i < epc_len; i++) {
//        if(this->epc[i] != tag2.epc[i]){
//            return false;
//        }
//    }
//    return true;
//}

//bool epc_tag::operator !=(const epc_tag &tag1, const epc_tag &tag2)
//{
//    return !(tag1 == tag2);
//}

//std::ostream &epc_tag::operator <<(std::ostream &out, const epc_tag &tag)
//{
//    out << tag.toString();
//    return out;
//}

class rfid_Impinj : public QThread
{
    Q_OBJECT
public:
    explicit rfid_Impinj(QObject *parent = 0);
    ~rfid_Impinj();
    bool connectReader(QString, quint16);
    bool disconnectReader();

    void setLog(logwindow *log);

    int getVersion();
    int getTemp();
    int getRegionFreq();
    int setOutputPower(int);
    int setOutputPower(int power0, int power1, int power2, int power3);
    int getOutputPower();
    int getWorkAntenna();
    int setWorkAntenna(int);

    void setIntervalSwitchAnt(int);
    int getIntervalSwitchAnt();

    int setReadFastSwitching(bool enable);

    void setRepeatRound(int);
    int getRepeatRound();

    int sendCommand(command);
    int sendCommand(quint8 *arr, int len);
    void sendTest();

    QString rssiToString(quint8);
    QString freqToString(quint8);
    int rssiToDbm(quint8);
    float freqToHz(quint8);

    bool getConnectStatus();

    void clearErrorFlag();
private:
    QTcpSocket * tcpSocket;
    logwindow *mLog;
    QTimer requestTimer;

    bool connectStatus;
    QByteArray buffHolder;

    void printLog(const QString&);

    // Fast switch ant attribute
    int interval;
    int repeat;

    // Decode variable
    qint32 state;
    quint8 checkSum;
    quint8 length;
    quint8 lengthCount;
    quint8 * cmdData;
    quint8 addr;
    quint8 cmd;

    bool errorFlag;

signals:
    void tagFound(epc_tag*);
    void connectStatusChanged(bool);
    void versionUpdated(QString);
    void tempUpdated(QString);
    void error(QString);
    void resetReader(bool);
    void outputPowerUpdate(quint8);
    void cmdDataArrival(quint8, quint8, quint8, quint8*);
public slots:
    void processDataArrival(quint8, quint8, quint8, quint8*);
    void processBuf();
    void processError(QAbstractSocket::SocketError socketError);
    void connectionStatus();

private slots:
    void requestTimerTimeOut();
};

#endif // RFID_IMPINJ_H
