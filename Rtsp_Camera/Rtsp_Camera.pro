QT += core
QT -= gui

CONFIG += c++11

TARGET = Rtsp_Camera
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    src/ffmpeg_h264.cpp \
    src/main.cpp \
    src/rtsp_client.cpp

HEADERS += \
    include/ffmpeg_h264.h \
    include/rtsp_client.h

INCLUDEPATH += ./include/

#OpenCV
INCLUDEPATH += /usr/local/include \
                /usr/local/include/opencv \
                /usr/local/include/opencv2

LIBS += /usr/local/lib/libopencv_highgui.so \
        /usr/local/lib/libopencv_core.so    \
        /usr/local/lib/libopencv_imgproc.so \
        /usr/local/lib/libopencv_imgcodecs.so

#bin file and dll
DESTDIR = $$PWD/bin
QMAKE_LFLAGS += "-Wl,-rpath=libs"

#FFMPEG include path
INCLUDEPATH +=  /usr/local/lib \
                /usr/local/include \
                /usr/include \
                /usr/include/include

#live555 include path
INCLUDEPATH +=  /usr/local/include/BasicUsageEnvironment \
                /usr/local/include/groupsock \
                /usr/local/include/liveMedia \
                /usr/local/include/UsageEnvironment


#FFMPEG libs
unix:!macx: LIBS += -L$$PWD/bin/libs/ -lavformat

INCLUDEPATH += $$PWD/bin/libs/
DEPENDPATH += $$PWD/bin/libs/

unix:!macx: LIBS += -L$$PWD/bin/libs/ -lavcodec

INCLUDEPATH += $$PWD/bin/libs/
DEPENDPATH += $$PWD/bin/libs/

unix:!macx: LIBS += -L$$PWD/bin/libs/ -lavutil

INCLUDEPATH += $$PWD/bin/libs/
DEPENDPATH += $$PWD/bin/libs/

unix:!macx: LIBS += -L$$PWD/bin/libs/ -lswscale

INCLUDEPATH += $$PWD/bin/libs/
DEPENDPATH += $$PWD/bin/libs/

unix:!macx: LIBS += -L$$PWD/bin/libs/ -lswresample

INCLUDEPATH += $$PWD/bin/libs/
DEPENDPATH += $$PWD/bin/libs/


#live555_libs

unix:!macx: LIBS += -L$$PWD/bin/libs/ -lliveMedia

INCLUDEPATH += $$PWD/bin/libs
DEPENDPATH += $$PWD/bin/libs

unix:!macx: PRE_TARGETDEPS += $$PWD/bin/libs/libliveMedia.a



unix:!macx: LIBS += -L$$PWD/bin/libs/ -lBasicUsageEnvironment

INCLUDEPATH += $$PWD/bin/libs
DEPENDPATH += $$PWD/bin/libs

unix:!macx: PRE_TARGETDEPS += $$PWD/bin/libs/libBasicUsageEnvironment.a



unix:!macx: LIBS += -L$$PWD/bin/libs/ -lgroupsock

INCLUDEPATH += $$PWD/bin/libs
DEPENDPATH += $$PWD/bin/libs

unix:!macx: PRE_TARGETDEPS += $$PWD/bin/libs/libgroupsock.a



unix:!macx: LIBS += -L$$PWD/bin/libs/ -lUsageEnvironment

INCLUDEPATH += $$PWD/bin/libs
DEPENDPATH += $$PWD/bin/libs

unix:!macx: PRE_TARGETDEPS += $$PWD/bin/libs/libUsageEnvironment.a
