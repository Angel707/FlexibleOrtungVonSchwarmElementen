#-------------------------------------------------
#
# Project created by QtCreator 2011-08-25T20:49:22
#
#-------------------------------------------------

QT       += core gui

TARGET = qt-calibration
TEMPLATE = app


SOURCES += main.cpp \
	cameracalibrationwidget.cpp \
	../Camera/Camera.cpp \
	../Camera/CamCalibration.cpp \
	../Camera/portopencvqt.cpp \
	../Camera/calibrationframe.cpp \
	matrixwidget.cpp

HEADERS  += \
	cameracalibrationwidget.h \
	../Camera/Camera.h \
	../Camera/CamCalibration.h \
	../Camera/portopencvqt.h \
	../Camera/calibrationframe.h \
	matrixwidget.h

FORMS    += cameracalibrationwidget.ui

CONFIG += link_pkgconfig
PKGCONFIG += opencv
