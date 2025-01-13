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
    AruCoAPI *arucoHandler;

    // View area
    QGraphicsScene *scene;
    QGraphicsView *view;
    QGraphicsPixmapItem *pixmapItem;

    // Status bar
    QLabel *mainStatusLabel;
    QLabel *currentTaskLabel;

    // Block Info
    QStandardItemModel *markerListModel;
    QStandardItem *item;

private:
    void initUI();

private slots:
    // Actions
    void showCameraInfo();
    void onDetectBlocksStateChanged(bool state);

    // GUI
    void updateFrame(const cv::Mat &frame);
    void updateCurrentTask(const QString &newTask);
    void onBlockDetected(const MarkerBlock &block);
};
#endif // MAINWINDOW_H
