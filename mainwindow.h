#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "capturethread.h"
#include <QCameraInfo>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
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
    void enableButtons(bool status);

private slots:
    // Actions
    void showCameraInfo();
    void openCamera();
    void updateDetectionStatus(bool status);
    void updateDistanceStatus(bool status);
    void updateCenterStatus(bool status);
    void beginCalibration(bool status);

    // GUI
    void updateFrame(cv::Mat *mat);
    void updateMarkersList(const QVector<QPair<int, double>> &markers);
};
#endif // MAINWINDOW_H
