#ifndef FILECOMPARE_H
#define FILECOMPARE_H

#include <QObject>
#include <QString>
#include <QByteArray>

class FileCompare : public QObject
{
	Q_OBJECT
public slots:
	void compare(const QString& first, const QString& second);
signals:
	void compared(const QString& first, const QString& second, bool);
private:
	static QByteArray gunzip(const std::string& filename);
};


#endif // FILECOMPARE_H
