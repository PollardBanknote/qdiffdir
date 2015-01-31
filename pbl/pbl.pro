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
    fileutils/abspath.cpp \
    fileutils/basename.cpp \
    fileutils/compare.cpp \
    fileutils/copyfile.cpp

HEADERS += \
    process/process.h \
    fileutils/fileutils.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
