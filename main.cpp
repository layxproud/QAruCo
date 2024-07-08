#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<QVector<QPair<int, double>>>("QVector<QPair<int,double>>");
    MainWindow w;
    w.show();
    return a.exec();
}
