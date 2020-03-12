TEMPLATE = app
TARGET = faceDetector

QT +=core gui multimedia
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

RESOURCES += \
        images.qrc


#OpenCV path
DEFINES += OPENCV_DATA_DIR=\\\"/usr/local/share/opencv4/\\\"

unix: !mac {
       INCLUDEPATH += /usr/local/include/opencv4
       LIBS += -L/usr/local/lib -lopencv_core -lopencv_imgproc -lopencv_video -lopencv_videoio -lopencv_imgcodecs -lopencv_objdetect -lopencv_face

   }
unix: mac {
         INCLUDEPATH += /path/to/opencv/include/opencv4
         LIBS += -L/path/to/opencv/lib -lopencv_world
     }
win32 {
       INCLUDEPATH += C:\opencv\build\include
       #LIBS += -lC:\opencv\build\x64\vc15\lib\opencv_world347d
       LIBS += -lC:\opencv\build\x64\vc15\lib\opencv_world347
   }


