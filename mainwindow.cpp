#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QGraphicsPixmapItem>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , workspace(new AruCoAPI(this))
    , markerListModel(new QStandardItemModel(this))
{
    ui->setupUi(this);
    initUI();
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
    currentTaskLabel->setText(tr("Нет задачи"));

    // Список меток
    ui->markersList->setModel(markerListModel);

    // connection from GUI
    connect(ui->cameraInfoAction, &QAction::triggered, this, &MainWindow::showCameraInfo);
    connect(ui->openCameraAction, &QAction::triggered, this, &MainWindow::openCamera);
    connect(ui->exitAction, &QAction::triggered, this, &MainWindow::close);
    connect(
        ui->detectMarkersAction,
        &QAction::triggered,
        workspace,
        &AruCoAPI::startMarkerDetectionTask);
    connect(
        ui->calculateDistanceAction,
        &QAction::triggered,
        workspace,
        &AruCoAPI::startDistanceCalculationTask);
    connect(ui->findCenterAction, &QAction::triggered, workspace, &AruCoAPI::startCenterFindingTask);
    connect(ui->cancelOperationsAction, &QAction::triggered, workspace, &AruCoAPI::cancelOperations);

    // connections to GUI
    connect(workspace, &AruCoAPI::frameReady, this, &MainWindow::updateFrame);
    connect(workspace, &AruCoAPI::taskChanged, this, &MainWindow::updateCurrentTask);
    connect(workspace, &AruCoAPI::distanceCalculated, this, &MainWindow::updateDistancesList);
    connect(workspace, &AruCoAPI::centerFound, this, &MainWindow::updateCenterList);
    connect(workspace, &AruCoAPI::newConfiguration, this, &MainWindow::updateConfigurationList);
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
    workspace->init();
}

void MainWindow::updateFrame(const cv::Mat &frame)
{
    cv::Mat frameCopy = frame.clone();
    QImage img(frameCopy.data, frameCopy.cols, frameCopy.rows, frameCopy.step, QImage::Format_RGB888);
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(QPixmap::fromImage(img.rgbSwapped()));
    scene->clear();
    scene->addItem(item);
    view->fitInView(item, Qt::KeepAspectRatio);
}

void MainWindow::updateDistancesList(const QVector<QPair<int, double>> &markers)
{
    markerListModel->clear();

    for (const auto &marker : markers) {
        QString text
            = QString("ID: %1, Расстояние: %2 mm").arg(marker.first).arg((float) marker.second);
        QStandardItem *item = new QStandardItem(text);
        markerListModel->appendRow(item);
    }
}

void MainWindow::updateCenterList(double distance)
{
    markerListModel->clear();
    QString text = QString("Расстояние до центра: %1 mm").arg(distance);
    QStandardItem *item = new QStandardItem(text);
    markerListModel->appendRow(item);
}

void MainWindow::updateConfigurationList(const Configuration &config)
{
    ui->configurationIdValue->setText(QString::fromStdString(config.id));
    ui->configurationDateValue->setText(QString::fromStdString(config.date));
    ui->configurationTypeValue->setText(QString::fromStdString(config.type));
    ui->configurationNameValue->setText(QString::fromStdString(config.name));
}

void MainWindow::updateCurrentTask(const QString &newTask)
{
    markerListModel->clear();
    currentTaskLabel->clear();
    currentTaskLabel->setText(newTask);
}
