#include <QGridLayout>
#include <QMenuBar>
#include <QStatusBar>
#include <QCameraInfo>
#include <QString>
#include <QMessageBox>
#include <QDebug>
#include <QApplication>
#include <QImage>
#include <QPixmap>
#include <QStandardPaths>
#include <QDir>

#include "mainwindow.h"
#include "utilites.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    camID(0),
    fileMenu(nullptr),
    capturer(nullptr)

{
    initUi();
    dataLock = new QMutex();
    camLock = new QMutex();
}

MainWindow::~MainWindow() {
    camLock->unlock();
    delete dataLock;
    delete camLock;
}

void MainWindow::initUi() {
    this->resize(1000, 800);

    fileMenu = menuBar()->addMenu("&File");

    QGridLayout *main_layout = new QGridLayout();

    imageScene = new QGraphicsScene(this);
    imageView = new QGraphicsView(imageScene);
    main_layout->addWidget(imageView, 0, 0, 12, 1);

    QGridLayout *tools_layout = new QGridLayout();
    main_layout->addLayout(tools_layout, 12, 0, 1, 1);
    monitorCheckBox = new QCheckBox(this);
    monitorCheckBox->setText("Monitor On/Off");
    tools_layout->addWidget(monitorCheckBox, 0, 0);

    recordButton = new QPushButton(this);
    recordButton->setText("Record");
    tools_layout->addWidget(recordButton, 0, 1, Qt::AlignCenter);
    tools_layout->addWidget(new QLabel(this), 0, 2);

    savedList = new QListView(this);
    savedList->setViewMode(QListView::IconMode);
    savedList->setResizeMode(QListView::Adjust);
    savedList->setSpacing(5);
    savedList->setWrapping(false);
    listModel = new QStandardItemModel(this);
    savedList->setModel(listModel);
    main_layout->addWidget(savedList, 13, 0, 4, 1);

    QWidget *widget = new QWidget();
    widget->setLayout(main_layout);
    setCentralWidget(widget);

    //status bar
    mainStatusBar = statusBar();
    mainStatusLabel = new QLabel(mainStatusBar);
    mainStatusBar->addPermanentWidget(mainStatusLabel);
    mainStatusLabel->setText("Watcher is Ready");

    connect(recordButton,SIGNAL(clicked(bool)), this, SLOT(recordingStartStop()));

    createActions();
    populateSavedList();
}

void MainWindow::createActions() {
    cameraInfoAction = new QAction("Camera &Information" ,this);
    fileMenu->addAction(cameraInfoAction);
    openCameraAction = new QAction("&Open Camera" ,this);
    fileMenu->addAction(openCameraAction);
    exitAction = new QAction("E&xit" ,this);
    fileMenu->addAction(exitAction);

    connect(cameraInfoAction, SIGNAL(triggered(bool)), this, SLOT(cameraInfo()));
    connect(exitAction, SIGNAL(triggered(bool)), QApplication::instance(), SLOT(quit()));
    connect(openCameraAction, SIGNAL(triggered(bool)), this, SLOT(openCamera()));

}
void MainWindow::cameraInfo() {
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    QString info = QString("Available Camera:  \n");

    foreach (const QCameraInfo &cameraInfo, cameras) {
        info += " - " + cameraInfo.deviceName() + "\n";
        //qDebug() << cameraInfo.deviceName() << "\n";
        info += cameraInfo.description() + "\n";
        //qDebug() << cameraInfo.description() << "\n";
    }
    QMessageBox::information(this, "Cameras", info);
}
void MainWindow::openCamera() {
    if (capturer != nullptr) {
        capturer->setRunning(false);
        disconnect(capturer, &CaptureThread::frameCaptured, this, &MainWindow::updateFrame);
        disconnect(capturer, &CaptureThread::fpsChanged, this, &MainWindow::updateFPS);
        disconnect(capturer, &CaptureThread::videoSaved, this, &MainWindow::appendSavedVideo);
        connect(capturer, &CaptureThread::finished, capturer, &CaptureThread::deleteLater);
    }
    capturer = new CaptureThread(camID, dataLock, camLock);
    connect(capturer, &CaptureThread::frameCaptured, this, &MainWindow::updateFrame);
    connect(capturer, &CaptureThread::fpsChanged, this, &MainWindow::updateFPS);
    connect(capturer, &CaptureThread::videoSaved, this, &MainWindow::appendSavedVideo);
    capturer->start();
    mainStatusLabel->setText(QString("Capturing Camera %1").arg(camID));
}
void MainWindow::updateFrame(cv::Mat *mat) {
    dataLock->lock();
    currentFrame = *mat;
    dataLock->unlock();

    QImage frame(currentFrame.data, currentFrame.cols, currentFrame.rows,
                 static_cast<int>(currentFrame.step), QImage::Format_BGR888);
    QPixmap image = QPixmap::fromImage(frame);
    imageScene->clear();
    imageView->resetTransform();
    imageScene->addPixmap(image);
    imageScene->update();
    imageView->setSceneRect(image.rect());
}
void MainWindow::updateFPS(float fps) {
    mainStatusLabel->setText(QString("FPS of current camera is %1").
                             arg(static_cast<int>(fps)));
}

void MainWindow::recordingStartStop() {
    QString text = recordButton->text();
    if (text == "Record" && capturer != nullptr) {
        capturer->setVideoSavingStatus(CaptureThread::STARTING);
        recordButton->setText("Stop Recording");
    } else if (text == "Stop Recording" && capturer != nullptr) {
        capturer->setVideoSavingStatus(CaptureThread::STOPPING);
        recordButton->setText("Record");
    }
}

void MainWindow::appendSavedVideo(QString name) {
    QString cover = Utilites::getSavedVideoPath(name, "jpg");
    QStandardItem *item = new QStandardItem;
    listModel->appendRow(item);
    QModelIndex index = listModel->indexFromItem(item);
    listModel->setData(index, QPixmap(cover).scaledToHeight(145), Qt::DecorationRole);
    listModel->setData(index, name, Qt::DisplayRole);
    savedList->scrollTo(index);
}

void MainWindow::populateSavedList() {
    QDir movieDir( Utilites::getDataPath());
    QStringList filters;
    filters << "*.jpg";
    QFileInfoList fileNames = movieDir.entryInfoList(filters,
                                            QDir::NoDotAndDotDot | QDir::Files, QDir::Name);
    foreach(QFileInfo file, fileNames) {
        QStandardItem *item = new QStandardItem;
        listModel->appendRow(item);
        QModelIndex index = listModel->indexFromItem(item);
        listModel->setData(index, QPixmap(file.absoluteFilePath()).scaledToHeight(145), Qt::DecorationRole);
        listModel->setData(index, file.baseName(), Qt::DisplayRole);
        savedList->scrollTo(index);
    }
}




