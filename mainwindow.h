#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMenu>
#include <QAction>
#include <QPushButton>
#include <QCheckBox>
#include <QListView>
#include <QLabel>
#include <QMutex>
#include <QStandardItemModel>

#include "opencv2/opencv.hpp"

#include "captureThread.h"

class MainWindow :public QMainWindow  {
    Q_OBJECT

public:
    explicit MainWindow (QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void cameraInfo();
    void openCamera();
    void updateFrame(cv::Mat*);
    void updateFPS(float fps);
    void recordingStartStop();
    void appendSavedVideo(QString name);

private:
    void populateSavedList();
    void initUi();
    void createActions();
    int camID;

    QMenu *fileMenu;

    QGraphicsScene *imageScene;
    QGraphicsView *imageView;

    QCheckBox *monitorCheckBox;
    QPushButton *recordButton;

    QListView *savedList;

    QStatusBar *mainStatusBar;
    QLabel *mainStatusLabel;

    QAction *cameraInfoAction;
    QAction *openCameraAction;
    QAction *exitAction;

    QStandardItemModel *listModel;

    cv::Mat currentFrame;
    QMutex *dataLock;
    QMutex *camLock;
    CaptureThread *capturer;
};

#endif // MAINWINDOW_H
