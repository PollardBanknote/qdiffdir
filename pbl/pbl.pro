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
    cpp/fs/abspath.cpp \
    cpp/fs/basename.cpp \
    cpp/fs/copyfile.cpp \
    cpp/fs/diriter.cpp \
    cpp/fs/cleanpath.cpp \
    cpp/fs/filestatus.cpp \
    cpp/fs/path.cpp \
    cpp/fs/direntry.cpp \
    cpp/fs/perms.cpp \
    cpp/fs/filetype.cpp \
    util/file.cpp

HEADERS += \
    cpp/fs/diriter.h \
    cpp/fs/filetype.h \
    cpp/fs/filestatus.h \
    cpp/fs/perms.h \
    cpp/fs/path.h \
    cpp/fs/direntry.h \
    cpp/fs/abspath.h \
    cpp/fs/basename.h \
    cpp/fs/copyfile.h \
    cpp/fs/cleanpath.h \
    process/detach.h \
    cpp/filesystem.h \
    util/file.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
