#-------------------------------------------------
#
# Project created by QtCreator 2015-01-31T09:08:43
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ui
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    comparewidget.cpp \
    dirdiffform.cpp \
    directorycontents.cpp \
    mysettings.cpp \
    settingsdialog.cpp \
    multilist.cpp

HEADERS  += mainwindow.h \
    comparewidget.h \
    dirdiffform.h \
    directorycontents.h \
    mysettings.h \
    settingsdialog.h \
    multilist.h

FORMS    += mainwindow.ui \
    comparewidget.ui \
    dirdiffform.ui \
    settingsdialog.ui \
    multilist.ui

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../pbl/release/ -lpbl
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../pbl/debug/ -lpbl
else:unix: LIBS += -L$$OUT_PWD/../pbl/ -lpbl

INCLUDEPATH += $$PWD/../pbl
DEPENDPATH += $$PWD/../pbl

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../pbl/release/libpbl.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../pbl/debug/libpbl.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../pbl/release/pbl.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../pbl/debug/pbl.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../pbl/libpbl.a
