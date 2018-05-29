#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    QFile file("log.txt");
//    file.open(QIODevice::WriteOnly | QIODevice::Text);


//    QDebug::QDebug(&file);

    a.setWindowIcon(QIcon(":/finishline.ico"));
    MainWindow w;
    w.setWindowFlags(Qt::WindowMinimizeButtonHint|Qt::WindowCloseButtonHint);
    w.show();

    return a.exec();
}
