#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "arucoapi.h"
#include <QCameraInfo>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLabel>
#include <QMainWindow>
#include <QStandardItemModel>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    AruCoAPI *workspace;

    // View area
    QGraphicsScene *scene;
    QGraphicsView *view;

    // Status bar
    QLabel *mainStatusLabel;
    QLabel *currentTaskLabel;

    // Video capturing
    cv::Mat currentFrame;
    QMutex *dataLock;
    CaptureThread *capturer;
    QStandardItemModel *listModel;

private:
    void initUI();

private slots:
    // Actions
    void showCameraInfo();
    void openCamera();

    // GUI
    void updateFrame(const cv::Mat &frame);
    void updateDistancesList(const QVector<QPair<int, double>> &markers);
    void updateCenterList(double distance);
    void updateCurrentTask(const QString &newTask);
};
#endif // MAINWINDOW_H
