#include <opencv2/imgproc.hpp>
#include "captureThread.h"

CaptureThread::CaptureThread(int camera, QMutex *lock) : running(false), cameraID(camera),
                    videoPath(""), data_lock(lock) {}

CaptureThread::CaptureThread(QString videoPath, QMutex *lock) : running(false), cameraID(-1),
                    videoPath(videoPath), data_lock(lock) {}

void CaptureThread::run() {
    running = true;
    cv::VideoCapture cap(cameraID);
    cv::Mat tmp_frame;
    while (running) {
        cap >> tmp_frame;
        if (tmp_frame.empty())
            break;
        data_lock->lock();
        frame = tmp_frame;
        data_lock->unlock();
        emit frameCaptured(&frame);
    }
    cap.release();
    running = false;
}

