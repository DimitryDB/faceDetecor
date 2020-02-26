#ifndef CAPTURETHREAD_H
#define CAPTURETHREAD_H
#include <QString>
#include <QThread>
#include <QMutex>

#include "opencv2/opencv.hpp"

class CaptureThread : public QThread {
    Q_OBJECT
public:
    CaptureThread(int camera, QMutex *lock);
    CaptureThread(QString videoPath, QMutex *lock);
    ~CaptureThread() override {}
    void setRunning(bool run) {running = run;}
protected:
    void run() override;

signals:
    void frameCaptured(cv::Mat *data);
private:
    bool running;
    int cameraID;
    QString videoPath;
    QMutex *data_lock;
    cv::Mat frame;

};


#endif // CAPTURETHREAD_H
