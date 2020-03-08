#include <QElapsedTimer>
#include <QDebug>
#include <QApplication>
#include <vector>
#include <opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>

#include "captureThread.h"
#include "utilites.h"

CaptureThread::CaptureThread(int camera, QMutex *lock, QMutex *cameraLock) : running(false), cameraID(camera),
                    videoPath(""), dataLock(lock), cameraLock(cameraLock), fps(0.0), cap(nullptr), playFile(false),
                    motionDetectingStatus(false), faceDetectingStatus(false), classifer(nullptr), frameWidth(0), frameHeight(0),
                    videoSavingStatus(STOPPED), savedVideoName(""), videoWriter(nullptr) {}

CaptureThread::CaptureThread(QString videoPath, QMutex *lock) : running(false), cameraID(-1),
                    videoPath(videoPath), dataLock(lock), cameraLock(nullptr), fps(0.0), cap(nullptr), playFile(true),
                    motionDetectingStatus(false), faceDetectingStatus(false), classifer(nullptr), frameWidth(0), frameHeight(0),
                    videoSavingStatus(STOPPED), savedVideoName(""), videoWriter(nullptr)  {}
CaptureThread::~CaptureThread() {
    if (cap != nullptr)
        delete cap;
    if (classifer != nullptr) {
        delete classifer;
    }
}

void CaptureThread::setMotionDetectingStatus(bool status) {
    motionDetectingStatus = status;
    motionDetected = false;
    if (videoSavingStatus != STOPPED)
        videoSavingStatus = STOPPING;
}
void CaptureThread::setFaceDetectingStatus(bool status) {
    faceDetectingStatus = status;
}
void CaptureThread::run() {
    int wait;
    running = true;
    if (playFile) {
        cap = new cv::VideoCapture(videoPath.toStdString());
        fps = cap->get(cv::CAP_PROP_FPS);
        wait  = 1000/fps;
    }
    else {
        cameraLock->tryLock(500);
        cap = new cv::VideoCapture(cameraID);
    }
    frameWidth = cap->get(cv::CAP_PROP_FRAME_WIDTH);
    frameHeight = cap->get(cv::CAP_PROP_FRAME_HEIGHT);
    //qDebug() << "fps " << fps << "\n";
    //qDebug() << "width " << frameWidth << "height " << frameHeight << "\n";
    segmentor = cv::createBackgroundSubtractorMOG2(500, 16, true);
    classifer = new cv::CascadeClassifier(OPENCV_DATA_DIR "haarcascades/haarcascade_frontalface_default.xml");
    markDetector = cv::face::createFacemarkLBF();
    QString modelData = QApplication::instance()->applicationDirPath() + "/data/lbfmodel.yaml";
    //qDebug() << modelData;
    markDetector->loadModel(modelData.toStdString());
    cv::Mat tmpFrame;
    QElapsedTimer timer;
    while (running) {
        timer.start();
        *cap >> tmpFrame;
        if (tmpFrame.empty())
            break;
        // detections
        if(motionDetectingStatus)
            motionDetect(tmpFrame);
        if(faceDetectingStatus)
            detectFaces(tmpFrame);
        // video recording
        if(videoSavingStatus == STARTING)
            startSavingVideo(tmpFrame);
        if(videoSavingStatus == STARTED)
            videoWriter->write(tmpFrame);
        if(videoSavingStatus == STOPPING)
            stopSavingVideo();

        dataLock->lock();
        frame = tmpFrame;
        dataLock->unlock();
        qint64 elapsed = timer.elapsed();
        if (playFile)
            msleep(wait - elapsed);
        else {
            fps = 1.0/(elapsed / 1000.0);
        }
        emit fpsChanged(fps);
        emit frameCaptured(&frame);
    }
    cap->release();
    running = false;
    if (!playFile)
        cameraLock->unlock();
}
void CaptureThread::startSavingVideo(cv::Mat &firstFrame) {
    savedVideoName = Utilites::newSavedVideoName();
    QString cover = Utilites::getSavedVideoPath(savedVideoName, "jpg");
    cv::imwrite(cover.toStdString(), firstFrame);
    videoWriter = new cv::VideoWriter(
                Utilites::getSavedVideoPath(savedVideoName, "avi").toStdString(),
                cv::VideoWriter::fourcc('M','J','P','G'),
                fps ? fps : 25,
                cv::Size(frameWidth, frameHeight));
    videoSavingStatus = STARTED;
}
void CaptureThread::stopSavingVideo() {
    videoSavingStatus = STOPPED;
    videoWriter->release();
    delete videoWriter;
    videoWriter = nullptr;
    emit videoSaved(savedVideoName);
}
void CaptureThread::motionDetect(cv::Mat &frame) {
    cv::Mat fgmask;
    segmentor->apply(frame, fgmask);
    if (fgmask.empty())
        return;
    cv::threshold(fgmask, fgmask, 25, 255, cv::THRESH_BINARY);
    int kernelSize = 9;
    cv::Mat  kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernelSize, kernelSize));
    cv::erode(fgmask, fgmask, kernel);
    cv::dilate(fgmask, fgmask, kernel, cv::Point(-1,-1), 3);
    std::vector<std::vector<cv::Point>> counturs;
    cv::findContours(fgmask, counturs, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    bool hasMotion = counturs.size() > 0;
    if (!motionDetected && hasMotion) {
        motionDetected = true;
        setVideoSavingStatus(STARTING);
        //qDebug() << "motion" ;
    } else if(motionDetected && !hasMotion) {
        motionDetected = false;
        setVideoSavingStatus(STOPPING);
        //qDebug() << "motion ended";
    }
    // rectangle around
    cv::Scalar color = cv::Scalar(0,0,255);
    for (size_t i = 0; i < counturs.size(); i++) {
        cv::Rect rect = cv::boundingRect(counturs[i]);
        cv::rectangle(frame, rect, color, 1);
    }
}
void CaptureThread::detectFaces(cv::Mat &frame) {
    std::vector<cv::Rect> faces;
    cv::Mat grayFrame;
    cv::cvtColor(frame, grayFrame, cv::COLOR_RGB2GRAY);
    classifer->detectMultiScale(grayFrame, faces, 1.3, 5);
    cv::Scalar color = cv::Scalar(0,255,0);
    for (size_t i = 0; i < faces.size(); i++) {
        cv::rectangle(frame, faces[i], color, 1);
    }
    std::vector<std::vector<cv::Point2f>> shapes;
    if (markDetector->fit(frame,faces,shapes)) {
        for (unsigned long i = 0; i < faces.size(); i++) {
            for (unsigned long k = 0; k < shapes[i].size(); k++) {
                //cv::circle(frame, shapes[i][k], 2, color, cv::FILLED);
                QString index = QString("%1").arg(k);
                cv::putText(frame, index.toStdString(), shapes[i][k], cv::FONT_HERSHEY_SIMPLEX, 0.25, color, 1);
            }
        }
    }
}




