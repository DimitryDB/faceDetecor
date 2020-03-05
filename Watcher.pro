TEMPLATE = app
TARGET = Watcher

QT +=core gui multimedia network concurrent
greaterThan(QT_MAJOR_VERSION, 4): QT+= widgets

CONFIG += c++11

HEADERS += \
        captureThread.h \
        mainwindow.h \
        utilites.h
SOURCES += \
        captureThread.cpp \
        main.cpp \
        mainwindow.cpp \
        utilites.cpp


#OpenCV path
unix: !mac {
       INCLUDEPATH += /usr/local/include/opencv4
       LIBS += -L/usr/local/include/opencv4 -lopencv_core -lopencv_imgproc -lopencv_video -lopencv_videoio -lopencv_imgcodecs
   }
unix: mac {
         INCLUDEPATH += /path/to/opencv/include/opencv4
         LIBS += -L/path/to/opencv/lib -lopencv_world -lopencv_imgcodecs
     }
win32 {
       INCLUDEPATH += C:\opencv\build\include
       #LIBS += -lC:\opencv\build\x64\vc15\lib\opencv_world347d
       LIBS += -lC:\opencv\build\x64\vc15\lib\opencv_world347
   }

