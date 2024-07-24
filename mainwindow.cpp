#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QGraphicsPixmapItem>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , workspace(new Workspace(this))
    , listModel(new QStandardItemModel(this))
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
    ui->markersList->setModel(listModel);

    // connection from GUI
    connect(ui->cameraInfoAction, &QAction::triggered, this, &MainWindow::showCameraInfo);
    connect(ui->openCameraAction, &QAction::triggered, this, &MainWindow::openCamera);
    connect(ui->exitAction, &QAction::triggered, this, &MainWindow::close);
    connect(
        ui->detectMarkersAction,
        &QAction::triggered,
        workspace,
        &Workspace::startMarkerDetectionTask);
    connect(
        ui->calculateDistanceAction,
        &QAction::triggered,
        workspace,
        &Workspace::startDistanceCalculationTask);
    connect(ui->findCenterAction, &QAction::triggered, workspace, &Workspace::startCenterFindingTask);
    connect(ui->cancelOperationsAction, &QAction::triggered, workspace, &Workspace::cancelOperations);

    // connections to GUI
    connect(workspace, &Workspace::frameReady, this, &MainWindow::updateFrame);
    connect(workspace, &Workspace::taskChanged, this, &MainWindow::updateCurrentTask);
    connect(workspace, &Workspace::distanceCalculated, this, &MainWindow::updateDistancesList);
    connect(workspace, &Workspace::centerFound, this, &MainWindow::updateCenterList);
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
    listModel->clear();

    for (const auto &marker : markers) {
        QString text = QString("ID: %1, Расстояние: %2 mm")
                           .arg(marker.first)
                           .arg((float) marker.second * 100);
        QStandardItem *item = new QStandardItem(text);
        listModel->appendRow(item);
    }
}

void MainWindow::updateCenterList(double distance)
{
    listModel->clear();
    QString text = QString("Расстояние до центра: %1 mm").arg(distance * 100.0);
    QStandardItem *item = new QStandardItem(text);
    listModel->appendRow(item);
}

void MainWindow::updateCurrentTask(const QString &newTask)
{
    listModel->clear();
    currentTaskLabel->clear();
    currentTaskLabel->setText(newTask);
}
