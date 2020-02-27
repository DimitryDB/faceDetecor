#include <QElapsedTimer>
#include <QDebug>
#include <opencv2/imgproc.hpp>
#include "captureThread.h"

CaptureThread::CaptureThread(int camera, QMutex *lock) : running(false), cameraID(camera),
                    videoPath(""), data_lock(lock), fps(0.0) {}

CaptureThread::CaptureThread(QString videoPath, QMutex *lock) : running(false), cameraID(-1),
                    videoPath(videoPath), data_lock(lock) {}

void CaptureThread::run() {
    running = true;
    cv::VideoCapture cap(cameraID);
    cv::Mat tmp_frame;
    QElapsedTimer timer;
    while (running) {
        timer.start();
        cap >> tmp_frame;
        if (tmp_frame.empty())
            break;
        data_lock->lock();
        frame = tmp_frame;
        data_lock->unlock();
        qint64 elapsed = timer.elapsed();
        fps = 1.0/(elapsed / 1000.0);
        emit fpsChanged(fps);
        emit frameCaptured(&frame);
    }
    cap.release();
    running = false;
}


