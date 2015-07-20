#ifndef MYSETTINGS_H
#define MYSETTINGS_H

class QString;
class QSettings;

class MySettings
{
public:
    static MySettings& instance();

    QString getDiffTool() const;
    QString getDiffToolSetting() const;
    void setDiffTool(const QString&);
private:
    MySettings();
    ~MySettings();

    void setValue(const QString& key, const QString& value);

    QSettings* store;
};

#endif // MYSETTINGS_H
