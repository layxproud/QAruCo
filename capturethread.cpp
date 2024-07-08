#include "capturethread.h"
#include <QDebug>

CaptureThread::CaptureThread(int camera, QMutex *lock)
    : running(false)
    , cameraID(camera)
    , dataLock(lock)
    , markerDetectionStatus(false)
    , distanceCalculatingStatus(false)
    , centerFindingStatus(false)
    , calibrationStatus(false)
{}

CaptureThread::~CaptureThread() {}

bool CaptureThread::loadCalibrationParameters(const std::string &filename)
{
    cv::FileStorage fs(filename, cv::FileStorage::READ);
    if (!fs.isOpened())
        return false;
    fs["CameraMatrix"] >> cameraMatrix;
    fs["DistCoeffs"] >> distCoeffs;
    fs.release();
    return true;
}

void CaptureThread::run()
{
    running = true;
    cv::VideoCapture cap(cameraID);
    if (!cap.isOpened()) {
        qCritical() << "Не удалось открыть камеру";
        return;
    }

    if (!loadCalibrationParameters("calibration.yml")) {
        qCritical() << "Не удалось загрузить калибровочные параметры";
        return;
    }

    cv::Mat tmpFrame;
    AruCoDict = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
    detectorParams = cv::aruco::DetectorParameters();
    detector = cv::aruco::ArucoDetector(AruCoDict, detectorParams);

    while (running) {
        cap >> tmpFrame;
        if (tmpFrame.empty())
            break;

        // Detect Markers
        std::vector<int> markerIds;
        std::vector<std::vector<cv::Point2f>> markerCorners;
        if (markerDetectionStatus || distanceCalculatingStatus || centerFindingStatus) {
            detectMarkers(tmpFrame, markerIds, markerCorners);
            drawDetectedMarkers(tmpFrame, markerCorners, markerIds);
        }

        if (distanceCalculatingStatus) {
            calculateDistance(markerCorners, markerIds);
        }

        if (centerFindingStatus) {
            findCenter(tmpFrame, markerCorners, markerIds);
        }

        dataLock->lock();
        frame = tmpFrame;
        dataLock->unlock();
        emit frameCaptured(&frame);
    }

    cap.release();
    running = false;
}

void CaptureThread::detectMarkers(
    cv::Mat &frame,
    std::vector<int> &markerIds,
    std::vector<std::vector<cv::Point2f>> &markerCorners)
{
    std::vector<std::vector<cv::Point2f>> rejectedCorners;
    detector.detectMarkers(frame, markerCorners, markerIds, rejectedCorners);
}

void CaptureThread::drawDetectedMarkers(
    cv::Mat &frame,
    const std::vector<std::vector<cv::Point2f>> &markerCorners,
    const std::vector<int> &markerIds)
{
    if (!markerIds.empty()) {
        cv::aruco::drawDetectedMarkers(frame, markerCorners, markerIds);
    }
}

void CaptureThread::calculateDistance(
    const std::vector<std::vector<cv::Point2f>> &markerCorners, const std::vector<int> &markerIds)
{
    if (markerIds.empty())
        return;

    int nMarkers = markerCorners.size();
    std::vector<cv::Vec3d> rvecs(nMarkers), tvecs(nMarkers);
    float markerSize = 48.0;

    cv::Mat objPoints(4, 1, CV_32FC3);
    objPoints.ptr<cv::Vec3f>(0)[0] = cv::Vec3f(-markerSize / 2.f, markerSize / 2.f, 0);
    objPoints.ptr<cv::Vec3f>(0)[1] = cv::Vec3f(markerSize / 2.f, markerSize / 2.f, 0);
    objPoints.ptr<cv::Vec3f>(0)[2] = cv::Vec3f(markerSize / 2.f, -markerSize / 2.f, 0);
    objPoints.ptr<cv::Vec3f>(0)[3] = cv::Vec3f(-markerSize / 2.f, -markerSize / 2.f, 0);

    cv::aruco::estimatePoseSingleMarkers(
        markerCorners, markerSize, cameraMatrix, distCoeffs, rvecs, tvecs);

    QVector<QPair<int, double>> markers;

    for (int i = 0; i < nMarkers; i++) {
        cv::solvePnP(
            objPoints, markerCorners.at(i), cameraMatrix, distCoeffs, rvecs.at(i), tvecs.at(i));

        double distance = std::sqrt(
            tvecs.at(i)[0] * tvecs.at(i)[0] + tvecs.at(i)[1] * tvecs.at(i)[1]
            + tvecs.at(i)[2] * tvecs.at(i)[2]);

        markers.append(qMakePair(markerIds.at(i), distance));
    }

    std::sort(
        markers.begin(),
        markers.end(),
        [](const QPair<int, double> &a, const QPair<int, double> &b) { return a.first < b.first; });

    emit markersDetected(markers);
}

void CaptureThread::findCenter(
    cv::Mat &frame,
    const std::vector<std::vector<cv::Point2f>> &markerCorners,
    const std::vector<int> &markerIds)
{
    // Implement center finding logic here
}
