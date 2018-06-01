#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <QPlainTextEdit>

class logwindow: public QObject
{
    Q_OBJECT
public:
    explicit logwindow(QString path, QPlainTextEdit *view);
    ~logwindow();
    void appendMsg(const QString&);
private:
    QFile mLogFile;
    QPlainTextEdit *mView;

signals:

public slots:

};

#endif // LOGWINDOW_H
