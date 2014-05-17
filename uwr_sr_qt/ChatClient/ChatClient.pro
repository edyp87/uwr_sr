TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
QT += network
HEADERS += chatclient.h ../ChatServer/chatserver.h
SOURCES += main.cpp chatclient.cpp ../ChatServer/chatserver.cpp
