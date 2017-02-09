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
    cpp/fs/basename.cpp \
    cpp/fs/copyfile.cpp \
    cpp/fs/diriter.cpp \
    cpp/fs/cleanpath.cpp \
    cpp/fs/filestatus.cpp \
    cpp/fs/path.cpp \
    cpp/fs/direntry.cpp \
    cpp/fs/perms.cpp \
    cpp/fs/filetype.cpp \
    util/strings.cpp \
    cpp/fs/absolute.cpp \
    cpp/fs/create_directory.cpp \
    cpp/fs/remove.cpp \
    cpp/fs/tempdir.cpp \
    cpp/fs/current_path.cpp \
    fileutil/compare.cpp

HEADERS += \
    cpp/fs/diriter.h \
    cpp/fs/filetype.h \
    cpp/fs/filestatus.h \
    cpp/fs/perms.h \
    cpp/fs/path.h \
    cpp/fs/direntry.h \
    cpp/fs/basename.h \
    cpp/fs/copyfile.h \
    cpp/fs/cleanpath.h \
    process/detach.h \
    cpp/filesystem.h \
    cpp/version.h \
    util/strings.h \
    cpp/fs/absolute.h \
    cpp/os.h \
    cpp/fs/create_directory.h \
    cpp/fs/remove.h \
    cpp/fs/tempdir.h \
    cpp/fs/current_path.h \
    fileutil/compare.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
