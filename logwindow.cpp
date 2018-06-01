#include "logwindow.h"
#include <QScrollBar>

logwindow::logwindow(QString path, QPlainTextEdit *view)
{
    mView = view;
}

logwindow::~logwindow()
{

}

void logwindow::appendMsg(const QString &msg)
{
    mView->appendPlainText(msg);
//    mView->verticalScrollBar()->setValue(mView->verticalScrollBar()->maximum());
}
