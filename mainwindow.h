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
#include <QHBoxLayout>

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
    void openFile();
    void closeVideoStream();
    void updateFrame(cv::Mat*);
    void updateFPS(float fps);
    void recordingStartStop();
    void appendSavedVideo(QString name);
    void playVideoFromLib(const QModelIndex &index);
    void updateMonitorStatus(int status);

private:
    void populateSavedList();
    void initUi();
    void createActions();
    void playCaptureSetup();
    int camID;

    QMenu *fileMenu;

    QGraphicsScene *imageScene;
    QGraphicsView *imageView;

    QCheckBox *monitorCheckBox;
    QPushButton *recordButton;

    QListView *savedList;

    QStatusBar *mainStatusBar;
    QLabel *mainStatusLabel;
    QLabel *fpsStatusLabel;

    QAction *cameraInfoAction;
    QAction *openCameraAction;
    QAction *openFileAction;
    QAction *closeVideoStreamAction;
    QAction *exitAction;


    QStandardItemModel *listModel;

    cv::Mat currentFrame;
    QMutex *dataLock;
    QMutex *camLock;
    CaptureThread *capturer;
};

#endif // MAINWINDOW_H
