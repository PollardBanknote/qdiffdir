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
    fs/copyfile.cpp \
    fs/diriter.cpp \
    fs/cleanpath.cpp \
    fs/filestatus.cpp \
    fs/path.cpp \
    fs/direntry.cpp \
    fs/file.cpp \
    fs/perms.cpp \
    fs/filetype.cpp

HEADERS += \
    fs/diriter.h \
    fs/filetype.h \
    fs/filestatus.h \
    fs/perms.h \
    fs/path.h \
    fs/direntry.h \
    fs/file.h \
    fs/abspath.h \
    fs/basename.h \
    fs/copyfile.h \
    fs/cleanpath.h \
    process/detach.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
