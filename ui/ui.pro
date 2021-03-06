#-------------------------------------------------
#
# Project created by QtCreator 2015-01-31T09:08:43
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qdiffdir
DESTDIR = ../
TEMPLATE = app

QMAKE_CXXFLAGS = -pipe
QMAKE_CXXFLAGS_DEBUG = -Og -ggdb3
QMAKE_CXXFLAGS_RELEASE = -O2
QMAKE_CXXFLAGS_WARN_OFF = warnoff
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wsign-compare -Wconversion -Wpointer-arith -Winit-self \
    -Wcast-qual -Wredundant-decls -Wcast-align -Wwrite-strings  -Wno-long-long \
    -Woverloaded-virtual -Wformat -Wno-unknown-pragmas -Wnon-virtual-dtor \
    -Wno-c++0x-compat

SOURCES += main.cpp\
        mainwindow.cpp \
    dirdiffform.cpp \
    mysettings.cpp \
    settingsdialog.cpp \
    filenamematcher.cpp \
    filecompare.cpp \
    comparisonlist.cpp \
    editmatchruledialog.cpp

HEADERS  += mainwindow.h \
    dirdiffform.h \
    mysettings.h \
    settingsdialog.h \
    compare.h \
    matcher.h \
    filenamematcher.h \
    filecompare.h \
    comparisonlist.h \
    editmatchruledialog.h

FORMS    += mainwindow.ui \
    dirdiffform.ui \
    settingsdialog.ui \
    editmatchruledialog.ui

INCLUDEPATH += $$PWD/..

LIBS += -L$$OUT_PWD/../qutility/ -lqutility
DEPENDPATH += $$PWD/../qutility
PRE_TARGETDEPS += $$OUT_PWD/../qutility/libqutility.a

LIBS += -L$$OUT_PWD/../pbl/ -lpbl
DEPENDPATH += $$PWD/../pbl
PRE_TARGETDEPS += $$OUT_PWD/../pbl/libpbl.a

LIBS += -L$$OUT_PWD/../cpp/ -lcpp
DEPENDPATH += $$PWD/../cpp
PRE_TARGETDEPS += $$OUT_PWD/../cpp/libcpp.a
