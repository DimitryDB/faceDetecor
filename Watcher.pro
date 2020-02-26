TEMPLATE = app
TARGET = Watcher

QT +=core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT+= widgets

CONFIG += c++11

HEADERS += \
        captureThread.h \
        mainwindow.h
SOURCES += \
        captureThread.cpp \
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
       INCLUDEPATH += C:\opencv\build\include
       LIBS += -lC:\opencv\build\x64\vc15\lib\opencv_world347
   }

