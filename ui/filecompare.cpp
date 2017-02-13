#include "filecompare.h"

#include <QProcess>
#include <QFile>

#include "fileutil/compare.h"
#include "util/strings.h"
#include "qutilities/convert.h"

void FileCompare::compare(
	const QString& first,
	const QString& second
)
{
	bool res;

	/// @todo Support different archive types (ex., bz2)
	if ( first.endsWith(".gz") || second.endsWith(".gz"))
	{
		/// @todo use popen and compare streams
		const QByteArray data1 = gunzip(first.toStdString());
		const QByteArray data2 = gunzip(second.toStdString());

		res = ( data1 == data2 );
	}
	else
	{
		res = ( pbl::fs::compare(first.toStdString(), second.toStdString()) == 1 );
	}

	emit compared(first, second, res);
}

QByteArray FileCompare::gunzip(const std::string& filename)
{
	if ( pbl::ends_with(filename, ".gz"))
	{
		QStringList l;
		l << "-c" << qt::convert(filename);

		QProcess gz;
		gz.start("gunzip", l);

		if ( !gz.waitForFinished())
		{
			return QByteArray();
		}

		return gz.readAllStandardOutput();
	}
	else
	{
		QFile file(qt::convert(filename));

		if ( !file.open(QIODevice::ReadOnly))
		{
			return QByteArray();
		}

		return file.readAll();
	}
}
