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
    dirdiffform.cpp \
    directorycontents.cpp \
    mysettings.cpp \
    settingsdialog.cpp \
    multilist.cpp \
    directorycomparison.cpp \
    workerthread.cpp \
    qutilities/icons.cpp

HEADERS  += mainwindow.h \
    dirdiffform.h \
    directorycontents.h \
    mysettings.h \
    settingsdialog.h \
    multilist.h \
    directorycomparison.h \
    compare.h \
    items.h \
    workerthread.h \
    matcher.h \
    qutilities/icons.h

FORMS    += mainwindow.ui \
    dirdiffform.ui \
    settingsdialog.ui

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
