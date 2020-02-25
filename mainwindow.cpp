#include <QGridLayout>
#include <QMenuBar>
#include <QStatusBar>
#include <QCameraInfo>
#include <QString>
#include <QMessageBox>
#include <QDebug>
#include "mainwindow.h"


MainWindow::MainWindow(QWidget *parent) {
    initUi();
}

MainWindow::~MainWindow() {}

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
    main_layout->addWidget(savedList, 13, 0, 4, 1);

    QWidget *widget = new QWidget();
    widget->setLayout(main_layout);
    setCentralWidget(widget);

    //status bar
    mainStatusBar = statusBar();
    mainStatusLabel = new QLabel(mainStatusBar);
    mainStatusBar->addPermanentWidget(mainStatusLabel);
    mainStatusLabel->setText("Watcher is Ready");

    createActions();
}

void MainWindow::createActions() {
    cameraInfoAction = new QAction("Camera &Information" ,this);
    fileMenu->addAction(cameraInfoAction);
    openCameraAction = new QAction("&Open Camera" ,this);
    fileMenu->addAction(openCameraAction);
    exitAction = new QAction("E&xit" ,this);
    fileMenu->addAction(exitAction);

    connect(cameraInfoAction, SIGNAL(triggered(bool)), this, SLOT(cameraInfo()));

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


