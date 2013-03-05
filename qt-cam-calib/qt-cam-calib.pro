#-------------------------------------------------
#
# Project created by QtCreator 2011-08-21T19:54:44
#
#-------------------------------------------------

QT       += core gui

TARGET = qt-cam-calib
TEMPLATE = app


SOURCES += main.cpp \
		mainwindow.cpp \
	../Camera/Camera.cpp \
	../Camera/CamCalibration.cpp \
	../Camera/calibrationframe.cpp \
	../Camera/cameraview.cpp \
	../Camera/birdseyeview.cpp \
	../Camera/camerapixmap.cpp \
	../Camera/cvlibwrapper.cpp

HEADERS  += mainwindow.h \
	../Camera/Camera.h \
	../Camera/CamCalibration.h \
	../Camera/calibrationframe.h \
	../Camera/cameraview.h \
	../Camera/birdseyeview.h \
	../Camera/camerapixmap.h \
	../Camera/cvlibwrapper.h

FORMS    += mainwindow.ui

CONFIG += link_pkgconfig
PKGCONFIG += opencv
