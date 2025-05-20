# Created by and for Qt Creator This file was created for editing the project sources only.
# You may attempt to use it for building too, by modifying this file here.

#TARGET = WebSocketDemo

QT = core gui widgets qml websockets

RESOURCES += $$PWD/src/resources.qrc

HEADERS = \
   $$PWD/src/state/Atom.h \
   $$PWD/src/state/AtomObserverWrapper.h \
   $$PWD/src/state/DerivedAtom.h \
   $$PWD/src/state/IAtomObserver.h \
   $$PWD/src/websockets/WebSocketAction.h \
   $$PWD/src/websockets/WebSocketCmComs.h \
   $$PWD/src/websockets/WebSocketPubSub.h \
   $$PWD/src/websockets/WebSocketTopics.h

SOURCES = \
   $$PWD/src/state/Atom.cpp \
   $$PWD/src/state/AtomObserverWrapper.cpp \
   $$PWD/src/state/DerivedAtom.cpp \
   $$PWD/src/websockets/WebSocketAction.cpp \
   $$PWD/src/websockets/WebSocketCmComs.cpp \
   $$PWD/src/websockets/WebSocketPubSub.cpp \
   $$PWD/src/main.cpp

INCLUDEPATH = \
    $$PWD/src/state \
    $$PWD/src/websockets

#DEFINES = 

DISTFILES += \
    src/qml/components/StatusControl.qml \
    src/qml/components/StatusControlArray.qml \
    src/qml/main.qml

