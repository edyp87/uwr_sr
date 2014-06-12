#-------------------------------------------------
#
# Project created by Marek Jenda
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = uwr_sr_qt
TEMPLATE = app


SOURCES += main.cpp\
    chatclient.cpp \
    chatserver.cpp

HEADERS  += chatserver.h \
    chatclient.h
