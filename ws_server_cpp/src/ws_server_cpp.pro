# Created by and for Qt Creator This file was created for editing the project sources only.
# You may attempt to use it for building too, by modifying this file here.

#TARGET = ws_server_cpp

QT = core gui widgets websockets

CONFIG += console

HEADERS = \
   $$PWD/websockets/WebSocketBroker.h

SOURCES = \
   $$PWD/websockets/WebSocketBroker.cpp \
   $$PWD/main.cpp

INCLUDEPATH = \
    $$PWD/websockets

#DEFINES = 

