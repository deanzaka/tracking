#-------------------------------------------------
#
# Project created by QtCreator 2014-10-24T04:51:28
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = colorTracking
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += \
    colorTracking.cpp

INCLUDEPATH += /usr/local/include/opencv
LIBS += -L/usr/local/lib
LIBS += -lopencv_core
LIBS += -lopencv_imgproc
LIBS += -lopencv_highgui
LIBS += -lopencv_ml
LIBS += -lopencv_video
LIBS += -lopencv_features2d
LIBS += -lopencv_calib3d
LIBS += -lopencv_objdetect
LIBS += -lopencv_contrib
LIBS += -lopencv_legacy
LIBS += -lopencv_flann
LIBS += -lopencv_nonfree
