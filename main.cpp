#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QDebug>

#include <windows.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    QFile file("log.txt");
//    file.open(QIODevice::WriteOnly | QIODevice::Text);

//    FreeConsole();

//    // create a separate new console window
//    AllocConsole();

//    // attach the new console to this application's process
//    AttachConsole(GetCurrentProcessId());

//    // reopen the std I/O streams to redirect I/O to the new console
//    freopen("CON", "w", stdout);
//    freopen("CON", "w", stderr);
//    freopen("CON", "r", stdin);
//    QDebug::QDebug(&file);

    a.setWindowIcon(QIcon(":/finishline.ico"));
    MainWindow w;
    w.setWindowFlags(Qt::WindowMinimizeButtonHint|Qt::WindowCloseButtonHint);
    w.show();

    return a.exec();
}
