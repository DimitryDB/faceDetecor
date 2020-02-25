TEMPLATE = app
TARGET = Watcher

QT +=core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT+= widgets

CONFIG += c++11

HEADERS += \
        mainwindow.h
SOURCES += \
        main.cpp \
        mainwindow.cpp \


#OpenCV path
unix: !mac {
       INCLUDEPATH += /usr/local/include/opencv4
       LIBS += -L/usr/local/include/opencv4 -lopencv_core -lopencv_imgproc -lopencv_video -lopencv_videoio
   }
unix: mac {
         INCLUDEPATH += /path/to/opencv/include/opencv4
         LIBS += -L/path/to/opencv/lib -lopencv_world
     }
win32 {
       INCLUDEPATH += c:/path/to/opencv/include/opencv4
       LIBS += -lc:/path/to/opencv/lib/opencv_world
   }

