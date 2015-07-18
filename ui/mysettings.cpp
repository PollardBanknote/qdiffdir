#include "mysettings.h"

#include <QSettings>
#include <QString>

const char difftool_key[] = "difftool";
const char filemanager_key[] = "filemanager";

MySettings& MySettings::instance()
{
    static MySettings settings;
    return settings;
}

QString MySettings::getDiffTool() const
{
    QString s = getDiffToolSetting();
    if (s.isEmpty())
        s = "kompare";
    return s;
}

QString MySettings::getDiffToolSetting() const
{
    return store->value(difftool_key).toString();
}

void MySettings::setDiffTool(const QString & s)
{
    setValue(difftool_key, s);
}

QString MySettings::getFileManager() const
{
    QString s = getFileManagerSetting();
    if (s.isEmpty())
        s = "dolphin";
    return s;
}

QString MySettings::getFileManagerSetting() const
{
    return store->value(filemanager_key).toString();
}

void MySettings::setFileManager(const QString & s)
{
    setValue(filemanager_key, s);
}

MySettings::MySettings()
{
    store = new QSettings("Pollard Banknote", "qdiffdir");
}

MySettings::~MySettings()
{
    delete store;
}

void MySettings::setValue(const QString &key, const QString &value)
{
    if (value.isEmpty())
        store->remove(key);
    else
        store->setValue(key, value);
}
