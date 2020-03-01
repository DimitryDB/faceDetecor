#ifndef UTILITES_H
#define UTILITES_H

#include <QString>

class Utilites {

public:
    static QString getDataPath();
    static QString newSavedVideoName();
    static QString getSavedVideoPath(QString name, QString postfix);
    static void delay(int seconds);
};

#endif // UTILITES_H
