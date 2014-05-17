#-------------------------------------------------
#
# Project created by QtCreator 2014-05-15T19:03:24
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = sr
CONFIG   += console
CONFIG   -= app_bundle
CONFIG   += qt

TEMPLATE = app


SOURCES += main.cpp \
    messages.pb.cc

HEADERS += \
    messages.pb.h

QT           += network

LIBS += -L/usr/lib/ -lboost_filesystem -lboost_system -lboost_thread -lprotobuf -lzmq
