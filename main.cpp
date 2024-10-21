#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<QVector<QPair<int, double>>>("QVector<QPair<int,double>>");
    qRegisterMetaType<Configuration>("Configuration");
    qRegisterMetaType<cv::Mat>("cv::Mat");
    MainWindow w;
    w.show();
    return a.exec();
}
