#-------------------------------------------------
#
# Project created by QtCreator 2017-02-13T13:58:11
#
#-------------------------------------------------

QT       -= core gui

TARGET = cpp
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    fs/absolute.cpp \
    fs/basename.cpp \
    fs/cleanpath.cpp \
    fs/copyfile.cpp \
    fs/create_directory.cpp \
    fs/current_path.cpp \
    fs/direntry.cpp \
    fs/diriter.cpp \
    fs/filestatus.cpp \
    fs/filetype.cpp \
    fs/path.cpp \
    fs/perms.cpp \
    fs/remove.cpp \
    fs/tempdir.cpp

HEADERS += \
    filesystem.h \
    version.h \
    fs/absolute.h \
    fs/basename.h \
    fs/cleanpath.h \
    fs/copyfile.h \
    fs/create_directory.h \
    fs/current_path.h \
    fs/direntry.h \
    fs/diriter.h \
    fs/filestatus.h \
    fs/filetype.h \
    fs/path.h \
    fs/perms.h \
    fs/remove.h \
    fs/tempdir.h \
    config/os.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
