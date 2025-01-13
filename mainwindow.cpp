#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QGraphicsPixmapItem>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , pixmapItem(new QGraphicsPixmapItem())
    , arucoHandler(new AruCoAPI(this))
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

    // Camera feed
    scene = new QGraphicsScene(this);
    view = new QGraphicsView(scene);
    scene->addItem(pixmapItem);
    ui->cameraLayout->addWidget(view);

    // Status bar
    mainStatusLabel = new QLabel(ui->statusbar);
    ui->statusbar->addPermanentWidget(mainStatusLabel);
    currentTaskLabel = new QLabel(ui->statusbar);
    ui->statusbar->addPermanentWidget(currentTaskLabel);
    mainStatusLabel->setText(tr("QAruCo ready"));
    currentTaskLabel->setText(tr("No task"));

    // Information Panel
    ui->markersList->setModel(markerListModel);

    // connection from GUI
    connect(ui->cameraInfoAction, &QAction::triggered, this, &MainWindow::showCameraInfo);
    connect(ui->exitAction, &QAction::triggered, this, &MainWindow::close);
    connect(
        ui->detectBlocksAction, &QAction::triggered, this, &MainWindow::onDetectBlocksStateChanged);

    // connections to GUI
    connect(arucoHandler, &AruCoAPI::taskChanged, this, &MainWindow::updateCurrentTask);
    connect(arucoHandler, &AruCoAPI::frameReady, this, &MainWindow::updateFrame);
    connect(arucoHandler, &AruCoAPI::blockDetected, this, &MainWindow::onBlockDetected);
}

void MainWindow::showCameraInfo()
{
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    QString info = tr("Avaliable cameras: \n");

    foreach (const QCameraInfo &cameraInfo, cameras) {
        info += " - " + cameraInfo.deviceName() + ": ";
        info += cameraInfo.description() + "\n";
    }
    QMessageBox::information(this, "Cameras info", info);
}

void MainWindow::onDetectBlocksStateChanged(bool state)
{
    markerListModel->clear();
    pixmapItem->setPixmap({});
    ui->configurationIdValue->setText("");
    ui->configurationDateValue->setText("");
    ui->configurationTypeValue->setText("");
    ui->configurationNameValue->setText("");

    arucoHandler->detectMarkerBlocks(state);
}

void MainWindow::updateFrame(const cv::Mat &frame)
{
    QImage img(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(img.rgbSwapped());
    pixmapItem->setPixmap(pixmap);
    view->update();
}

void MainWindow::updateCurrentTask(const QString &newTask)
{
    markerListModel->clear();
    currentTaskLabel->clear();
    currentTaskLabel->setText(newTask);
}

void MainWindow::onBlockDetected(const MarkerBlock &block)
{
    markerListModel->clear();
    QString text;

    text = QString("Coordinates: %1; %2").arg(block.blockCenter.x()).arg(block.blockCenter.y());
    item = new QStandardItem(text);
    markerListModel->appendRow(item);

    text = QString("Distance: %1").arg(block.distanceToCenter);
    item = new QStandardItem(text);
    markerListModel->appendRow(item);

    text = QString("Angle: %1").arg(block.blockAngle);
    item = new QStandardItem(text);
    markerListModel->appendRow(item);

    ui->configurationIdValue->setText(QString::fromStdString(block.config.id));
    ui->configurationDateValue->setText(QString::fromStdString(block.config.date));
    ui->configurationTypeValue->setText(QString::fromStdString(block.config.type));
    ui->configurationNameValue->setText(QString::fromStdString(block.config.name));
}
