#ifndef WORKSPACE_H
#define WORKSPACE_H

#include "capturethread.h"
#include "yamlhandler.h"
#include <opencv2/opencv.hpp>
#include <QObject>

class Workspace : public QObject
{
    Q_OBJECT
public:
    explicit Workspace(QObject *parent = nullptr);
    ~Workspace();

    void init();
    void startThread(QThread *thread);
    void stopThread(QThread *thread);

signals:
    void frameReady(const cv::Mat &frame);
    void taskChanged(const QString &newTask);
    void taskFinished(bool success, const QString &message);
    void distanceCalculated(const QVector<QPair<int, double>> &markers);
    void centerFound(double distance);

public slots:
    void startMarkerDetectionTask();
    void startDistanceCalculationTask();
    void startCenterFindingTask();
    void cancelOperations();

private:
    YamlHandler *yamlHandler;
    CaptureThread *captureThread;
};

#endif // WORKSPACE_H
