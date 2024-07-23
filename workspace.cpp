#include "workspace.h"
#include <QDebug>

Workspace::Workspace(QObject *parent)
    : QObject{parent}
    , yamlHandler(new YamlHandler(this))
    , captureThread(new CaptureThread(this))
{
    connect(captureThread, &CaptureThread::frameReady, this, &Workspace::frameReady);
    connect(captureThread, &CaptureThread::distanceCalculated, this, &Workspace::distanceCalculated);
}

Workspace::~Workspace()
{
    stopThread(captureThread);
}

void Workspace::init()
{
    CalibrationParams calibrationParams;
    if (!yamlHandler->loadCalibrationParameters("calibration.yml", calibrationParams)) {
        emit taskFinished(false, tr("Не обнаружен файл калибровки. Сначала откалибруйте камеру!"));
        return;
    }
    captureThread->setCalibrationParams(calibrationParams);
    captureThread->setYamlHandler(yamlHandler);
    startThread(captureThread);
    qDebug() << "THREAD STARTED";
}

void Workspace::startThread(QThread *thread)
{
    if (thread && !thread->isRunning()) {
        thread->start();
    }
}

void Workspace::stopThread(QThread *thread)
{
    if (thread && thread->isRunning()) {
        CaptureThread *capture = dynamic_cast<CaptureThread *>(thread);
        if (capture) {
            capture->stop();
            capture->wait();
            return;
        }

        thread->quit();
        thread->wait();
    }
}

void Workspace::startMarkerDetectionTask()
{
    if (!captureThread->getMarkerDetectionStatus()) {
        captureThread->setDistanceCalculatingStatus(false);
        captureThread->setCenterFindingStatus(false);
        captureThread->setMarkerDetectingStatus(true);
        emit taskChanged(tr("Ведется поиск маркеров"));
    }
}

void Workspace::startDistanceCalculationTask()
{
    if (!captureThread->getDistanceCalculatingStatus()) {
        captureThread->setCenterFindingStatus(false);
        captureThread->setMarkerDetectingStatus(false);
        captureThread->setDistanceCalculatingStatus(true);
        emit taskChanged(tr("Ведется измерение расстояний"));
    }
}

void Workspace::startCenterFindingTask()
{
    if (!captureThread->getCenterFindingStatus()) {
        captureThread->setDistanceCalculatingStatus(false);
        captureThread->setMarkerDetectingStatus(false);
        captureThread->setCenterFindingStatus(true);
        emit taskChanged(tr("Ведется поиск центра"));
    }
}
