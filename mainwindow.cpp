#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , capturer(nullptr)
{
    ui->setupUi(this);
    initUI();
    enableButtons(false);
    dataLock = new QMutex();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initUI()
{
    this->resize(1000, 800);

    // Фид камеры
    scene = new QGraphicsScene(this);
    view = new QGraphicsView(scene);
    ui->cameraLayout->addWidget(view);

    // Статус бар
    mainStatusLabel = new QLabel(ui->statusbar);
    ui->statusbar->addPermanentWidget(mainStatusLabel);
    currentTaskLabel = new QLabel(ui->statusbar);
    ui->statusbar->addPermanentWidget(currentTaskLabel);
    mainStatusLabel->setText(tr("QAruCo готов к работе"));

    // Список меток
    listModel = new QStandardItemModel(this);
    ui->markersList->setModel(listModel);

    connect(ui->cameraInfoAction, &QAction::triggered, this, &MainWindow::showCameraInfo);
    connect(ui->openCameraAction, &QAction::triggered, this, &MainWindow::openCamera);
    connect(ui->exitAction, &QAction::triggered, this, &MainWindow::close);
    connect(ui->detectMarkersAction, &QAction::triggered, this, &MainWindow::updateDetectionStatus);
    connect(ui->calculateDistanceAction, &QAction::triggered, this, &MainWindow::updateDistanceStatus);
    connect(ui->findCenterAction, &QAction::triggered, this, &MainWindow::updateCenterStatus);
    connect(ui->calibrateAction, &QAction::triggered, this, &MainWindow::beginCalibration);
}

void MainWindow::enableButtons(bool status)
{
    ui->detectMarkersAction->setEnabled(status);
    ui->calculateDistanceAction->setEnabled(status);
    ui->findCenterAction->setEnabled(status);
    ui->calibrateAction->setEnabled(status);

    if (!status) {
        ui->detectMarkersAction->setChecked(false);
        updateDetectionStatus(false);
        ui->calculateDistanceAction->setChecked(false);
        updateDistanceStatus(false);
        ui->findCenterAction->setChecked(false);
        updateCenterStatus(false);
    }
}

void MainWindow::showCameraInfo()
{
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    QString info = tr("Доступные камеры: \n");

    foreach (const QCameraInfo &cameraInfo, cameras) {
        info += "  - " + cameraInfo.deviceName() + ": ";
        info += cameraInfo.description() + "\n";
    }
    QMessageBox::information(this, "Список камер", info);
}

void MainWindow::openCamera()
{
    if (capturer != nullptr) {
        mainStatusLabel->setText(tr("Видеопоток остановлен"));
        ui->openCameraAction->setText(tr("Старт"));
        enableButtons(false);

        capturer->setRunning(false);
        disconnect(capturer, &CaptureThread::frameCaptured, this, &MainWindow::updateFrame);
        disconnect(capturer, &CaptureThread::markersDetected, this, &MainWindow::updateMarkersList);
        connect(capturer, &CaptureThread::finished, capturer, &CaptureThread::deleteLater);
        capturer = nullptr;

        scene->clear();
        scene->update();
        return;
    }

    int camID = 0;
    capturer = new CaptureThread(camID, dataLock);
    connect(capturer, &CaptureThread::frameCaptured, this, &MainWindow::updateFrame);
    connect(capturer, &CaptureThread::markersDetected, this, &MainWindow::updateMarkersList);
    capturer->start();
    mainStatusLabel->setText(QString(tr("Видеопоток с камеры %1 запущен")).arg(camID));
    ui->openCameraAction->setText(tr("Стоп"));
    enableButtons(true);
}

void MainWindow::updateDetectionStatus(bool status)
{
    if (capturer == nullptr) {
        ui->detectMarkersAction->setChecked(false);
        return;
    }
    if (status) {
        capturer->setMarkerDetectingStatus(true);
        currentTaskLabel->setText(tr("Поиск маркеров"));
    } else {
        capturer->setMarkerDetectingStatus(false);
        currentTaskLabel->setText(tr(""));
    }
}

void MainWindow::updateDistanceStatus(bool status)
{
    if (capturer == nullptr) {
        ui->calculateDistanceAction->setChecked(false);
        return;
    }
    capturer->setDistanceCalculatingStatus(status);
}

void MainWindow::updateCenterStatus(bool status)
{
    if (capturer == nullptr) {
        ui->findCenterAction->setChecked(false);
        return;
    }
    capturer->setCenterFindingStatus(status);
}

void MainWindow::beginCalibration(bool status)
{
    if (capturer == nullptr) {
        return;
    }
    capturer->setCalibrationStatus(status);
}

void MainWindow::updateFrame(cv::Mat *mat)
{
    dataLock->lock();
    currentFrame = *mat;
    dataLock->unlock();

    QImage frame(
        currentFrame.data,
        currentFrame.cols,
        currentFrame.rows,
        currentFrame.step,
        QImage::Format_RGB888);
    QPixmap image = QPixmap::fromImage(frame.rgbSwapped());

    scene->clear();
    view->resetMatrix();
    scene->addPixmap(image);
    scene->update();
    view->setSceneRect(image.rect());
}

void MainWindow::updateMarkersList(const QVector<QPair<int, double>> &markers)
{
    listModel->clear();

    for (const auto &marker : markers) {
        QString text = QString("ID: %1, Расстояние: %2 mm").arg(marker.first).arg(marker.second);
        QStandardItem *item = new QStandardItem(text);
        listModel->appendRow(item);
    }
}
