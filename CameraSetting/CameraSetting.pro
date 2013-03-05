#-------------------------------------------------
#
# Project created by QtCreator 2011-09-23T16:44:18
#
#-------------------------------------------------

QT       += core gui

TARGET = CameraSetting
TEMPLATE = app


SOURCES += main.cpp\
		camerasetting.cpp

HEADERS  += camerasetting.h

FORMS    += camerasetting.ui

CONFIG += link_pkgconfig
PKGCONFIG += opencv
