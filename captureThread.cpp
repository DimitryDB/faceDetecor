#include <QElapsedTimer>

#include <opencv2/imgproc.hpp>

#include "captureThread.h"
#include "utilites.h"

CaptureThread::CaptureThread(int camera, QMutex *lock, QMutex *cameraLock) : running(false), cameraID(camera),
                    videoPath(""), data_lock(lock), camera_lock(cameraLock), fps(0.0), cap(nullptr), playFile(false),
                    frame_width(0), frame_height(0), video_saving_status(STOPPED), saved_video_name(""),
                    video_writer(nullptr) {}

CaptureThread::CaptureThread(QString videoPath, QMutex *lock) : running(false), cameraID(-1),
                    videoPath(videoPath), data_lock(lock), camera_lock(nullptr), fps(0.0), cap(nullptr), playFile(true),
                    frame_width(0), frame_height(0), video_saving_status(STOPPED), saved_video_name(""),
                    video_writer(nullptr)  {}

void CaptureThread::run() {
    int wait;
    running = true;
    if (playFile) {
        cap = new cv::VideoCapture(videoPath.toStdString());
        fps = cap->get(cv::CAP_PROP_FPS);
        wait  = 1000/fps;
    }
    else {
        camera_lock->tryLock(500);
        cap = new cv::VideoCapture(cameraID);
    }
    frame_width = cap->get(cv::CAP_PROP_FRAME_WIDTH);
    frame_height = cap->get(cv::CAP_PROP_FRAME_HEIGHT);
    //qDebug() << "fps " << fps << "\n";
    //qDebug() << "width " << frame_width << "height " << frame_height << "\n";
    cv::Mat tmp_frame;
    QElapsedTimer timer;
    while (running) {
        timer.start();
        *cap >> tmp_frame;
        if (tmp_frame.empty())
            break;

        if(video_saving_status == STARTING)
            startSavingVideo(tmp_frame);
        if(video_saving_status == STARTED)
            video_writer->write(tmp_frame);
        if(video_saving_status == STOPPING)
            stopSavingVideo();

        data_lock->lock();
        frame = tmp_frame;
        data_lock->unlock();
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
        camera_lock->unlock();
    delete cap;
}

void CaptureThread::startSavingVideo(cv::Mat &firstFrame) {
    saved_video_name = Utilites::newSavedVideoName();
    QString cover = Utilites::getSavedVideoPath(saved_video_name, "jpg");

    cv::imwrite(cover.toStdString(), firstFrame);

    video_writer = new cv::VideoWriter(
                Utilites::getSavedVideoPath(saved_video_name, "avi").toStdString(),
                cv::VideoWriter::fourcc('M','J','P','G'),
                fps ? fps : 25,
                cv::Size(frame_width, frame_height));
    video_saving_status = STARTED;
}

void CaptureThread::stopSavingVideo() {
    video_saving_status = STOPPED;
    video_writer->release();
    delete video_writer;
    video_writer = nullptr;
    emit videoSaved(saved_video_name);
}


