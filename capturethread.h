#ifndef CAPTURETHREAD_H
#define CAPTURETHREAD_H

#include <opencv2/aruco.hpp>
#include <opencv2/opencv.hpp>
#include <QMutex>
#include <QObject>
#include <QThread>
#include <QTime>

class CaptureThread : public QThread
{
    Q_OBJECT
public:
    CaptureThread(int camera, QMutex *lock);
    ~CaptureThread();

    void setRunning(bool run) { running = run; }
    void setMarkerDetectingStatus(bool status) { markerDetectionStatus = status; }
    void setDistanceCalculatingStatus(bool status) { distanceCalculatingStatus = status; }
    void setCenterFindingStatus(bool status) { centerFindingStatus = status; }
    void setCalibrationStatus(bool status) { calibrationStatus = status; }

protected:
    void run() override;

signals:
    void frameCaptured(cv::Mat *data);
    void markersDetected(const QVector<QPair<int, double>> &markers);

private:
    bool running;
    int cameraID;
    QMutex *dataLock;
    cv::Mat frame;

    // Marker detection
    bool markerDetectionStatus;
    bool distanceCalculatingStatus;
    bool centerFindingStatus;
    bool calibrationStatus;
    cv::aruco::Dictionary AruCoDict;
    cv::aruco::DetectorParameters detectorParams;
    cv::aruco::ArucoDetector detector;

    // Calibration
    cv::Mat cameraMatrix;
    cv::Mat distCoeffs;

private:
    bool loadCalibrationParameters(const std::string &filename);

    void detectMarkers(
        cv::Mat &frame,
        std::vector<int> &markerIds,
        std::vector<std::vector<cv::Point2f>> &markerCorners);
    void drawDetectedMarkers(
        cv::Mat &frame,
        const std::vector<std::vector<cv::Point2f>> &markerCorners,
        const std::vector<int> &markerIds);
    void calculateDistance(const std::vector<std::vector<cv::Point2f>> &markerCorners,
        const std::vector<int> &markerIds);
    void findCenter(
        cv::Mat &frame,
        const std::vector<std::vector<cv::Point2f>> &markerCorners,
        const std::vector<int> &markerIds);
};

#endif // CAPTURETHREAD_H
