#-------------------------------------------------
#
# Project created by QtCreator 2015-01-31T09:08:23
#
#-------------------------------------------------

QT       -= gui

TARGET = pbl
TEMPLATE = lib
CONFIG += staticlib

QMAKE_CXXFLAGS = -pipe
QMAKE_CXXFLAGS_DEBUG = -Og -ggdb3
QMAKE_CXXFLAGS_RELEASE = -O2
QMAKE_CXXFLAGS_WARN_OFF = warnoff
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wsign-compare -Wconversion -Wpointer-arith -Winit-self \
    -Wcast-qual -Wredundant-decls -Wcast-align -Wwrite-strings  -Wno-long-long \
    -Woverloaded-virtual -Wformat -Wno-unknown-pragmas -Wnon-virtual-dtor \
    -Wno-c++0x-compat

SOURCES += \
    process/detach.cpp \
    util/strings.cpp \
    fileutil/compare.cpp \
    process/which.cpp

HEADERS += \
    process/detach.h \
    util/strings.h \
    fileutil/compare.h \
    process/which.h \
    config/os.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH += $$PWD/..

LIBS += -L$$OUT_PWD/../cpp/ -lcpp
DEPENDPATH += $$PWD/../cpp
PRE_TARGETDEPS += $$OUT_PWD/../cpp/libcpp.a
