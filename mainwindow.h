#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QTimer>
#include <QDateTime>
#include <QHash>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QSqlQueryModel>
#include <QLineEdit>
#include <QDebug>
#include <QItemDelegate>
#include <QSqlTableModel>
#include <rfid_impinj.h>
#include <logwindow.h>

namespace Ui {
class MainWindow;
}

class myModel : public QSqlQueryModel{
    Q_OBJECT
public:
    myModel(QObject *parent = 0): QSqlQueryModel(parent){}
    bool setData(const QModelIndex & index, const QVariant & value, int role)
    {
        if (role == Qt::EditRole)
        {
            qDebug() << value.toString();
        }
        return true;
    }
    Qt::ItemFlags flags(const QModelIndex & index){
         return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
    }
};

class myItem : public QItemDelegate {
    Q_OBJECT
public:
    explicit myItem(QObject *parent = 0):QItemDelegate(parent){

    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const{
        QString val = index.model()->data(index, Qt::EditRole).toString();
        QLineEdit *spinbox = static_cast<QLineEdit*>(editor);
        spinbox->setText(val);
    }
    QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const{
        qDebug() << "createEditor";
        Q_UNUSED(index);
        QLineEdit *editor = new QLineEdit(parent);
        editor->setGeometry(option.rect);
        return editor;
    }
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const{
        QLineEdit *spinbox = static_cast<QLineEdit*>(editor);
        model->setData(index, spinbox->text(), Qt::EditRole);
    }
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const{
        Q_UNUSED(index);
        editor->setGeometry(option.rect);
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
    enum EoperationMode {
        ILDE = -1,
        TESTMODE,
        NORMALMODE,
        COLLECTTAGMODE
    };
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void updateTable();

private slots:
    void on_btnConDis_clicked();
    void on_connectStatusChanged(bool);

    void on_btnGetOutputPower_clicked();

    void on_btnSetOutputPower_clicked();

    void on_btnStartTimer();

    void requestTimerTimeOut();

    void checkBoxHandler(bool);

    void tagFound(epc_tag*);

    void startRequest();

    void on_btnSetDelayTime_clicked();

    void on_btnSetOffsetTime_clicked();

    void on_btnResetReader_clicked();

    void readBasicInfo();

    void updateOutputPower(quint8);
    void changeOperationMode(QString);
    void tabIndexChange(int);

    void on_btnCollectTag_clicked();

    void on_btnSave2File_clicked();

    void on_btnResetReader_7_clicked();

private:
    Ui::MainWindow *ui;
    QThread *rfid_thread;
    QTimer requestTimer;

//    myModel *model;
    QSqlQueryModel *modelPrepare;
    QSqlQueryModel *model;
    QSqlTableModel *racerModel;
    rfid_Impinj *myReader;
    logwindow *mLog;
    myItem * mItem;

    int createDb();

    QTimer readBasicInfoTimer;

    QHash<QString, epc_tag> tagsHolder;

    QSqlDatabase mCacheDb;

    EoperationMode operationMode;

    bool mRunning;
    QDateTime mStartTime;
    QDateTime mStartCaptureTime;
    quint64 mSessionTime;
    int timerid;

    QTimer oneShotStartTimer;
    int delayStartTime;
    int requestInterval;

    void createTable();
    void mainTableViewSetHeader();
    void collectTableViewSetHeader();
    void enableUI(bool);
    void startCount();
    void stopCount();
protected:
    void timerEvent(QTimerEvent *);
};

#endif // MAINWINDOW_H
