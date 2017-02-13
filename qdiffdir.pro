TEMPLATE = subdirs

SUBDIRS += \
    pbl \
    ui \
    cpp

pbl.depends = cpp
ui.depends = pbl
