TEMPLATE = subdirs

SUBDIRS += \
    pbl \
    ui \
    cpp \
    qutility

pbl.depends = cpp
qutility.depends = pbl
ui.depends = pbl qutility
