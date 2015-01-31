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
	pbl/process/detach.cpp \
    pbl/fileutils/qutils.cpp \
    pbl/fileutils/copyfile.cpp \
    pbl/fileutils/abspath.cpp \
    pbl/fileutils/compare.cpp \
    pbl/fileutils/basename.cpp

HEADERS  += mainwindow.h \
	dirdiffform.h \
	comparewidget.h \
    pbl/fileutils/qutils.h \
    pbl/fileutils/fileutils.h \
    pbl/process/process.h

FORMS    += mainwindow.ui \
	dirdiffform.ui \
	comparewidget.ui

OTHER_FILES += \
	LICENSE.txt \
    INSTALL.txt
