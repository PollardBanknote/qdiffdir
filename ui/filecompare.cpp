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

#include <cstdio>

#include <QProcess>
#include <QFile>

#include "pbl/fileutil/compare.h"
#include "pbl/util/strings.h"
#include "qutility/convert.h"

void FileCompare::compare(
	const QString& first,
    const QString& second,
    long long filesizelimit // in megabytes
)
{
	std::FILE* file1;
	bool is_process1 = false;

	{
		const std::string p = qt::convert(first);
		if (first.endsWith(".gz"))
		{
			const std::string cmd = "gunzip -c " + p;
			file1 = ::popen(cmd.c_str(), "r");
			is_process1 = true;
		}
		else file1 = std::fopen(p.c_str(), "rb");
	}

	std::FILE* file2;
	bool is_process2 = false;

	{
		const std::string p = qt::convert(second);
		if (second.endsWith(".gz"))
		{
			const std::string cmd = "gunzip -c " + p;
			file2 = ::popen(cmd.c_str(), "r");
			is_process2 = true;
		}
		else file2 = std::fopen(p.c_str(), "rb");
	}

	const bool res = ( pbl::fs::compare(file1, file2, filesizelimit * 1024 * 1024) == pbl::fs::compare_equal );

	if (is_process1)
		::pclose(file1);
	else
		std::fclose(file1);

	if (is_process2)
		::pclose(file2);
	else
		std::fclose(file2);

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
