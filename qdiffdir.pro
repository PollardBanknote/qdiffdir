TEMPLATE = subdirs

SUBDIRS += \
    pbl \
    ui

ui.depends = pbl
