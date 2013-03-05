#-------------------------------------------------
#
# Project created by QtCreator 2011-09-10T14:24:19
#
#-------------------------------------------------

QT       += core gui

TARGET = qt-schwarmelement
TEMPLATE = app

SOURCES += main.cpp\
	schwarmelementwidget.cpp \
	../Camera/Camera.cpp \
	../Camera/CamCalibration.cpp \
	../Camera/cameraview.cpp \
	../Camera/schwarmelement.cpp \
	../Camera/robustmatcher.cpp \
	../Camera/camerapixmap.cpp \
	../Camera/schwarmelementitem.cpp \
	../Camera/cvlibwrapper.cpp \
	../Camera/drossosdebug.cpp \
	../CameraSetting/camerasetting.cpp \
	../Camera/birdseyeview.cpp \
	../Camera/schwarmelementdetection.cpp

HEADERS  += schwarmelementwidget.h \
	../Camera/Camera.h \
	../Camera/CamCalibration.h \
	../Camera/cameraview.h \
	../Camera/schwarmelement.h \
	../Camera/robustmatcher.h \
	../Camera/camerapixmap.h \
	../Camera/schwarmelementitem.h \
	../Camera/cvlibwrapper.h \
	../Camera/drossosdebug.h \
	../CameraSetting/camerasetting.h \
	../Camera/birdseyeview.h \
	../Camera/schwarmelementdetection.h

FORMS    += schwarmelementwidget.ui \
	../CameraSetting/camerasetting.ui

CONFIG += link_pkgconfig
PKGCONFIG += opencv
