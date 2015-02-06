#-------------------------------------------------
#
# Project created by QtCreator 2015-01-31T09:08:23
#
#-------------------------------------------------

QT       -= core gui

TARGET = pbl
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    process/detach.cpp \
    fs/abspath.cpp \
    fs/basename.cpp \
    fs/compare.cpp \
    fs/copyfile.cpp \
    fs/diriter.cpp \
    fs/fileinfo.cpp

HEADERS += \
    process/process.h \
    fs/fileutils.h \
    fs/diriter.h \
    fs/fileinfo.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
