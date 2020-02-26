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

private:
    void initUi();
    void createActions();

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

    cv::Mat currentFrame;
    QMutex *data_lock;
    CaptureThread *capturer;
};

#endif // MAINWINDOW_H
