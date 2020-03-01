#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QCoreApplication>
#include <QEventLoop>

#include "utilites.h"


QString Utilites::getDataPath()
{
    QString user_movie_path = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation)[0];
    QDir movieDir(user_movie_path);
    movieDir.mkdir("Watcher");
    return movieDir.absoluteFilePath("Watcher");
}

QString Utilites::newSavedVideoName()
{
    QDateTime time = QDateTime::currentDateTime();
    return time.toString("yyyy-MM-dd+HH:mm:ss");
}

QString Utilites::getSavedVideoPath(QString name, QString postfix)
{
    return QString("%1/%2.%3").arg(Utilites::getDataPath(),name,postfix);
}

void Utilites::delay(int seconds)
{
    QTime dieTime= QTime::currentTime().addSecs(seconds);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, seconds*100);
}
