/* Copyright (c) 2017, Pollard Banknote Limited
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation and/or
   other materials provided with the distribution.

   3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software without
   specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
   OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
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
