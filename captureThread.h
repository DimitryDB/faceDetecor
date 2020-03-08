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
    ~CaptureThread() override;

    void setMotionDetectingStatus(bool status);
    bool cameraLocked();
    void setRunning(bool run) {running = run;}
    void setVideoSavingStatus(VideoSavingStatus status) {
        videoSavingStatus = status; }

protected:
    void run() override;

signals:
    void frameCaptured(cv::Mat *data);
    void fpsChanged(float fps);
    void videoSaved(QString name);

private:
    void startSavingVideo(cv::Mat &firstFrame);
    void stopSavingVideo();
    void motionDetect(cv::Mat &frame);

    bool running;
    int cameraID;
    QString videoPath;
    QMutex *dataLock;
    QMutex *cameraLock;
    cv::Mat frame;
    float fps;
    cv::VideoCapture *cap;
    bool playFile;
    //motion detect
    bool motionDetectingStatus;
    bool motionDetected;
    cv::Ptr<cv::BackgroundSubtractorMOG2> segmentor;
    //saving
    int frameWidth, frameHeight;
    VideoSavingStatus videoSavingStatus;
    QString savedVideoName;
    cv::VideoWriter *videoWriter;
};

#endif // CAPTURETHREAD_H
