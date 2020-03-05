#ifndef CAPTURETHREAD_H
#define CAPTURETHREAD_H
#include <QString>
#include <QThread>
#include <QMutex>

#include "opencv2/opencv.hpp"
#include <opencv2/videoio.hpp>
#include "opencv2/video/background_segm.hpp"


class CaptureThread : public QThread {
    Q_OBJECT

public:
    enum VideoSavingStatus { STARTING, STARTED, STOPPING, STOPPED };

    CaptureThread(int camera, QMutex *lock, QMutex *cameraLock);
    CaptureThread(QString videoPath, QMutex *lock);
    ~CaptureThread() override {}

    bool cameraLocked();
    void setRunning(bool run) {running = run;}
    void setVideoSavingStatus(VideoSavingStatus status) {
        video_saving_status = status; }

protected:
    void run() override;

signals:
    void frameCaptured(cv::Mat *data);
    void fpsChanged(float fps);
    void videoSaved(QString name);

private:
    void startSavingVideo(cv::Mat &firstFrame);
    void stopSavingVideo();

    bool running;
    int cameraID;
    QString videoPath;
    QMutex *data_lock;
    QMutex *camera_lock;
    cv::Mat frame;
    float fps;
    cv::VideoCapture *cap;
    bool playFile;
    //saving
    int frame_width, frame_height;
    VideoSavingStatus video_saving_status;
    QString saved_video_name;
    cv::VideoWriter *video_writer;

};


#endif // CAPTURETHREAD_H
