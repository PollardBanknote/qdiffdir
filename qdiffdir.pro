#-------------------------------------------------
#
# Project created by QtCreator 2012-05-01T10:14:46
#
#-------------------------------------------------

QMAKE_CXXFLAGS += -ansi -pedantic -Wall -Wsign-compare -Wconversion -Wpointer-arith -Winit-self -Wcast-qual -Wredundant-decls -Wcast-align -Wwrite-strings -Wno-long-long -Wshadow -Wno-missing-field-initializers

QT       += core gui

TARGET = qdiffdir
TEMPLATE = app


SOURCES += main.cpp\
		mainwindow.cpp \
	comparewidget.cpp \
	dirdiffform.cpp \
	detach.cpp \
    fileutils/qutils.cpp \
    fileutils/copyfile.cpp \
    fileutils/abspath.cpp \
    fileutils/compare.cpp \
    fileutils/basename.cpp

HEADERS  += mainwindow.h \
	dirdiffform.h \
	comparewidget.h \
	detach.h \
    fileutils/qutils.h \
    fileutils/fileutils.h

FORMS    += mainwindow.ui \
	dirdiffform.ui \
	comparewidget.ui

OTHER_FILES += \
	LICENSE.txt \
    INSTALL.txt
