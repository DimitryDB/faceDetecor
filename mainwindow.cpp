#include <QGridLayout>
#include <QMenuBar>
#include <QStatusBar>
#include <QCameraInfo>
#include <QString>
#include <QMessageBox>
#include <QApplication>
#include <QImage>
#include <QPixmap>
#include <QStandardPaths>
#include <QDir>
#include <QFileDialog>
#include <QThread>
#include <QToolBar>
#include "mainwindow.h"
#include "utilites.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    camID(0),
    fileMenu(nullptr),
    capturer(nullptr) {
    initUi();
    dataLock = new QMutex();
    camLock = new QMutex();
}
MainWindow::~MainWindow() {
    // whait till thread done its job to avoid app crash
    if(capturer != nullptr && capturer->isRunning())
        QThread::msleep(2000);
    camLock->try_lock();
    camLock->unlock();
    dataLock->try_lock();
    dataLock->unlock();
    delete camLock;
    delete dataLock;
}
void MainWindow::initUi() {
    this->resize(1000, 1000);

    fileMenu = menuBar()->addMenu("&File");
    viewToolBar = addToolBar("View");
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

    //overlays
    QGridLayout *overlayLayout = new QGridLayout();
    main_layout->addLayout(overlayLayout, 13, 0, 1, 1);
    overlayLayout->addWidget(new QLabel("Overlays", this));
    for (int i = 0; i < CaptureThread::OVERLAYS_COUNT; i++) {
        overlayCheckboxes[i] = new QCheckBox(this);
        overlayLayout->addWidget(overlayCheckboxes[i], 0, i + 1);
        connect(overlayCheckboxes[i], SIGNAL(stateChanged(int)), this, SLOT(updateOverlay(int)));
    }
    overlayCheckboxes[0]->setText("Rectangle");
    overlayCheckboxes[1]->setText("Points");
    overlayCheckboxes[2]->setText("Glases");
    overlayCheckboxes[3]->setText("Mouse");

    savedList = new QListView(this);
    savedList->setViewMode(QListView::IconMode);
    savedList->setResizeMode(QListView::Adjust);
    savedList->setSpacing(5);
    savedList->setWrapping(false);
    listModel = new QStandardItemModel(this);
    savedList->setModel(listModel);
    main_layout->addWidget(savedList, 14, 0, 4, 1);

    QWidget *widget = new QWidget();
    widget->setLayout(main_layout);
    setCentralWidget(widget);

    //status bar
    mainStatusBar = statusBar();
    mainStatusLabel = new QLabel(mainStatusBar);
    fpsStatusLabel = new QLabel(mainStatusBar);
    mainStatusBar->addPermanentWidget(fpsStatusLabel);
    mainStatusBar->addWidget(mainStatusLabel);
    mainStatusLabel->setText("Ready");

    connect(recordButton,SIGNAL(clicked(bool)), this, SLOT(recordingStartStop()));
    connect(monitorCheckBox, SIGNAL(stateChanged(int)), this, SLOT(updateMonitorStatus(int)));



    createActions();
    populateSavedList();
}
void MainWindow::createActions() {
    cameraInfoAction = new QAction("Camera &Information" ,this);
    fileMenu->addAction(cameraInfoAction);
    openCameraAction = new QAction("Open &Camera" ,this);
    fileMenu->addAction(openCameraAction);
    openFileAction = new QAction("Open &File" ,this);
    fileMenu->addAction(openFileAction);
    closeVideoStreamAction = new QAction("&Close Video Stream" ,this);
    fileMenu->addAction(closeVideoStreamAction);
    exitAction = new QAction("E&xit" ,this);
    fileMenu->addAction(exitAction);
    zoomOutAction = new QAction("Zoom Out" ,this);
    viewToolBar->addAction(zoomOutAction);
    zoomInAction = new QAction("Zoom In" ,this);
    viewToolBar->addAction(zoomInAction);
    originalTransformAction = new QAction("&Reset Transform" ,this);
    viewToolBar->addAction(originalTransformAction);


    connect(cameraInfoAction, SIGNAL(triggered(bool)), this, SLOT(cameraInfo()));
    connect(exitAction, SIGNAL(triggered(bool)), QApplication::instance(), SLOT(quit()));
    connect(openCameraAction, SIGNAL(triggered(bool)), this, SLOT(openCamera()));
    connect(closeVideoStreamAction, SIGNAL(triggered(bool)), this, SLOT(closeVideoStream()));
    connect(openFileAction, SIGNAL(triggered(bool)), this, SLOT(openFile()));
    connect(zoomInAction, SIGNAL(triggered(bool)), this, SLOT(zoomIn()));
    connect(zoomOutAction, SIGNAL(triggered(bool)), this, SLOT(zoomOut()));
    connect(originalTransformAction, SIGNAL(triggered(bool)), this, SLOT(resetTransform()));

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
    closeVideoStream();
    capturer = new CaptureThread(camID, dataLock, camLock);
    playCaptureSetup();
    mainStatusLabel->setText(QString("Capturing Camera %1").arg(camID));
    recordButton->setEnabled(true);
    monitorCheckBox->setCheckState(Qt::Unchecked);
    for (int i = 0; i < CaptureThread::OVERLAYS_COUNT; i++)
        overlayCheckboxes[i]->setCheckState(Qt::Unchecked);
}
void MainWindow::openFile() {
    QFileDialog dialog(this);
    dialog.setWindowTitle("Open Video");
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("Videos(*.avi)"));
    QStringList filePaths;
    if (dialog.exec()) {
        filePaths = dialog.selectedFiles();
        QString filePath = filePaths.at(0);
        closeVideoStream();
        capturer = new CaptureThread(filePath, dataLock);
        playCaptureSetup();
        mainStatusLabel->setText(QString("Playing video %1").arg(filePaths.at(0)));
    }
    recordButton->setEnabled(true);
    monitorCheckBox->setCheckState(Qt::Unchecked);
    for (int i = 0; i < CaptureThread::OVERLAYS_COUNT; i++)
        overlayCheckboxes[i]->setCheckState(Qt::Unchecked);
}
void MainWindow::playCaptureSetup() {
    connect(capturer, &CaptureThread::frameCaptured, this, &MainWindow::updateFrame);
    connect(capturer, &CaptureThread::fpsChanged, this, &MainWindow::updateFPS);
    connect(capturer, &CaptureThread::videoSaved, this, &MainWindow::appendSavedVideo);
    capturer->start();
}
void MainWindow::closeVideoStream() {
    if (capturer != nullptr) {
        capturer->setRunning(false);
        disconnect(capturer, &CaptureThread::frameCaptured, this, &MainWindow::updateFrame);
        disconnect(capturer, &CaptureThread::fpsChanged, this, &MainWindow::updateFPS);
        disconnect(capturer, &CaptureThread::videoSaved, this, &MainWindow::appendSavedVideo);
        connect(capturer, &CaptureThread::finished, capturer, &CaptureThread::deleteLater);
        imageScene->clear();
        mainStatusLabel->setText("Watcher is Ready");
        capturer = nullptr;
    }
}
void MainWindow::updateFrame(cv::Mat *mat) {
    dataLock->lock();
    currentFrame = *mat;
    dataLock->unlock();

    QImage frame(currentFrame.data, currentFrame.cols, currentFrame.rows,
                 static_cast<int>(currentFrame.step), QImage::Format_BGR888);
    QPixmap image = QPixmap::fromImage(frame);
    imageScene->clear();
    //imageView->resetTransform();
    imageScene->addPixmap(image);
    imageScene->update();
    imageView->setSceneRect(image.rect());
}
void MainWindow::updateFPS(float fps) {
    fpsStatusLabel->setText(QString("FPS %1").
                             arg(static_cast<int>(fps)));
}
void MainWindow::recordingStartStop() {
    QString text = recordButton->text();
    if (text == "Record" && capturer != nullptr) {
        capturer->setVideoSavingStatus(CaptureThread::STARTING);
        recordButton->setText("Stop Recording");
        monitorCheckBox->setEnabled(false);
    } else if (text == "Stop Recording" && capturer != nullptr) {
        capturer->setVideoSavingStatus(CaptureThread::STOPPING);
        recordButton->setText("Record");
        monitorCheckBox->setEnabled(true);
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
void MainWindow::playVideoFromLib(const QModelIndex &index) {
    QString filename =Utilites::getDataPath() + "/" + listModel->itemFromIndex(index)->text() + ".avi";
    //qDebug() << filename << "\n";
    closeVideoStream();
    capturer = new CaptureThread(filename, dataLock);
    playCaptureSetup();
    mainStatusLabel->setText(QString("Playing video %1").arg(filename));
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
    connect(savedList, SIGNAL(doubleClicked(const QModelIndex &)), this,
                                            SLOT(playVideoFromLib(const QModelIndex &)));
}
void MainWindow::updateMonitorStatus(int status) {
    //qDebug() << "test";
    if(capturer == nullptr)
        return;
    if(status) {
        capturer->setMotionDetectingStatus(true);
        recordButton->setEnabled(false);
    } else {
        capturer->setMotionDetectingStatus(false);
        recordButton->setEnabled(true);
    }
}

void MainWindow::zoomIn() {
    imageView->scale(1.2,1.2);
}
void MainWindow::zoomOut() {
    imageView->scale(0.8,0.8);
}
void MainWindow::resetTransform() {
    imageView->resetTransform();
}
void MainWindow::updateOverlay(int status) {
    if(capturer == nullptr)
        return;
    QCheckBox *box = qobject_cast<QCheckBox*>(sender());
    for (int i = 0; i < CaptureThread::OVERLAYS_COUNT; i++) {
        if (overlayCheckboxes[i] == box) {
            capturer->updateOverlayFlag(static_cast<CaptureThread::OverlayType>(i), status !=0);
        }
    }
}



