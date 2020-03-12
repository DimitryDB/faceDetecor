#ifndef CAPTURETHREAD_H
#define CAPTURETHREAD_H
#include <QString>
#include <QThread>
#include <QMutex>



#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/face/facemark.hpp>


class CaptureThread : public QThread {
    Q_OBJECT

public:
    enum VideoSavingStatus { STARTING, STARTED, STOPPING, STOPPED };
    enum OverlayType  { RECTANGLE, POINTS, GLASES, MOUSE_NOSE, OVERLAYS_COUNT };

    CaptureThread(int camera, QMutex *lock, QMutex *cameraLock);
    CaptureThread(QString videoPath, QMutex *lock);
    ~CaptureThread() override;

    void setMotionDetectingStatus(bool status);
    void setFaceDetectingStatus(bool status);
    void setRunning(bool run) {running = run;}
    void setVideoSavingStatus(VideoSavingStatus status) {
        videoSavingStatus = status; }
    void updateOverlayFlag(OverlayType  type, bool onOff);

protected:
    void run() override;

signals:
    void frameCaptured(cv::Mat *data);
    void fpsChanged(float fps);
    void videoSaved(QString name);

private:
    bool isOverlayOn(OverlayType  type) {return (overlayFlag & (1 << type)) !=0; }
    void startSavingVideo(cv::Mat &firstFrame);
    void stopSavingVideo();
    void motionDetect(cv::Mat &frame);
    void detectFaces(cv::Mat &frame);
    void loadOverlays();
    void drawGlasses(cv::Mat &frame, std::vector<cv::Point2f> &marks);
    void drawMouse(cv::Mat &frame, std::vector<cv::Point2f> &marks);

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
    //face detect

    cv::CascadeClassifier *classifer;
    cv::Ptr<cv::face::Facemark> markDetector;
    //saving
    int frameWidth, frameHeight;
    VideoSavingStatus videoSavingStatus;
    QString savedVideoName;
    cv::VideoWriter *videoWriter;
    //overlays
    cv::Mat glasses;
    cv::Mat mouseNose;
    uint8_t overlayFlag;

};

#endif // CAPTURETHREAD_H
