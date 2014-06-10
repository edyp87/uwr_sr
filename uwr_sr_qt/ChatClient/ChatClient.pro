TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += .
QT += core gui network widgets
HEADERS += chatclient.h ../ChatServer/chatserver.h
SOURCES += main.cpp chatclient.cpp ../ChatServer/chatserver.cpp
