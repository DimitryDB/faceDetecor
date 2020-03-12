#include <QElapsedTimer>
#include <QDebug>
#include <QApplication>
#include <QImage>
#include <cmath>

#include <opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>

#include "captureThread.h"
#include "utilites.h"

CaptureThread::CaptureThread(int camera, QMutex *lock, QMutex *cameraLock) : running(false), cameraID(camera),
                    videoPath(""), dataLock(lock), cameraLock(cameraLock), fps(0.0), cap(nullptr), playFile(false),
                    motionDetectingStatus(false), motionDetected(false),  classifer(nullptr),
                    frameWidth(0), frameHeight(0), videoSavingStatus(STOPPED), savedVideoName(""),
                    videoWriter(nullptr), overlayFlag(0) {
    loadOverlays();
}

CaptureThread::CaptureThread(QString videoPath, QMutex *lock) : running(false), cameraID(-1),
                    videoPath(videoPath), dataLock(lock), cameraLock(nullptr), fps(0.0), cap(nullptr), playFile(true),
                    motionDetectingStatus(false), motionDetected(false),  classifer(nullptr),
                    frameWidth(0), frameHeight(0), videoSavingStatus(STOPPED), savedVideoName(""),
                    videoWriter(nullptr), overlayFlag(0)  {
    loadOverlays();
}
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
void CaptureThread::updateOverlayFlag(CaptureThread::OverlayType  type, bool onOff) {
    uint8_t bit = 1 <<type;
    if (onOff)
        overlayFlag |= bit;
    else
        overlayFlag &= ~bit;
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
        if(overlayFlag > 0)
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
    if (isOverlayOn(RECTANGLE)) {
        for (size_t i = 0; i < faces.size(); i++)
            cv::rectangle(frame, faces[i], color, 1);
    }
    if (isOverlayOn(POINTS) || isOverlayOn(GLASES) || isOverlayOn(MOUSE_NOSE)) {
        std::vector<std::vector<cv::Point2f>> shapes;
        if (markDetector->fit(frame,faces,shapes)) {
            for (unsigned long i = 0; i < faces.size(); i++) {
                if (isOverlayOn(GLASES))
                    drawGlasses(frame, shapes[i]);
                if (isOverlayOn(MOUSE_NOSE))
                    drawMouse(frame, shapes[i]);
                if (isOverlayOn(POINTS)) {
                    for (unsigned long k = 0; k < shapes[i].size(); k++) {
                        QString index = QString("%1").arg(k);
                        cv::putText(frame, index.toStdString(), shapes[i][k], cv::FONT_HERSHEY_SIMPLEX, 0.25, color, 1);

                    }
                }
            }
        }
    }
}
void CaptureThread::loadOverlays() {
    QImage image;
    image.load(":/images/glasses.jpg");
    image = image.convertToFormat(QImage::Format_BGR888);
    glasses = cv::Mat(image.height(), image.width(), CV_8UC3,
                      image.bits(), image.bytesPerLine()).clone();
    image.load(":/images/mouse-nose.jpg");
    image = image.convertToFormat(QImage::Format_BGR888);
    mouseNose = cv::Mat(image.height(), image.width(), CV_8UC3,
                      image.bits(), image.bytesPerLine()).clone();

}
void CaptureThread::drawGlasses(cv::Mat &frame, std::vector<cv::Point2f> &marks) {
    cv::Mat tmp, mask;
    double distance = cv::norm(marks[45] - marks[36]) * 1.5;
    double scaleFactor = distance / glasses.cols;
    cv::resize(glasses, tmp, cv::Size(0,0), scaleFactor, scaleFactor, cv::INTER_NEAREST );

    double angle = -std::atan((marks[45].y - marks[36].y) / (marks[45].x - marks[36].x));
    cv::Point2f center = cv::Point(tmp.cols/2, tmp.cols/2);
    cv::Mat rotateMatrix = cv::getRotationMatrix2D(center, angle * 180/M_PI, 1.0);

    cv::Mat rotated;
    cv::warpAffine(tmp, rotated, rotateMatrix, tmp.size(),
                   cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255));

    center = cv::Point((marks[45].x + marks[36].x) /2, (marks[45].y + marks[36].y) /2 );
    cv::Rect rec(center.x - rotated.cols/2, center.y - rotated.rows/2 , rotated.cols, rotated.rows);
    cv::threshold(rotated, mask, 100, 255, cv::THRESH_BINARY);
    frame(rec) &= mask;
    rotated = cv::Scalar(0, 0, 255);
    rotated &= ~mask;
    frame(rec) |= rotated;
}
void CaptureThread::drawMouse(cv::Mat &frame, std::vector<cv::Point2f> &marks) {
    cv::Mat tmp;
    double distance = cv::norm(marks[13] - marks[3]) * 1.2;
    double scaleFactor = distance / glasses.cols;
    cv::resize(mouseNose, tmp, cv::Size(0,0), scaleFactor, scaleFactor, cv::INTER_NEAREST );

    double angle = -std::atan((marks[16].y - marks[0].y) / (marks[16].x - marks[0].x));
    cv::Point2f center = cv::Point(tmp.cols/2, tmp.cols/2);
    cv::Mat rotateMatrix = cv::getRotationMatrix2D(center, angle * 180/M_PI, 1.0);

    cv::Mat rotated;
    cv::warpAffine(tmp, rotated, rotateMatrix, tmp.size(),
                   cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255));

    center = cv::Point(marks[30].x, marks[30].y);
    cv::Rect rec(center.x - rotated.cols/2, center.y - rotated.rows/2 , rotated.cols, rotated.rows);
    frame(rec) &= rotated;
}






