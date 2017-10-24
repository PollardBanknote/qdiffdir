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

namespace
{
class FileOrProcess
{
public:
	FileOrProcess(const QString& filename, const QString& command)
	    : is_process(!command.isEmpty())
	{
		if ( is_process )
		{
			const std::string cmd = qt::convert(command + " " + filename);
			file       = ::popen(cmd.c_str(), "r");
		}
		else
		{
			const std::string p = qt::convert(filename);
			file = std::fopen(p.c_str(), "rb");
		}
	}

	~FileOrProcess()
	{
		if ( is_process )
		{
			::pclose(file);
		}
		else
		{
			std::fclose(file);
		}
	}

	std::FILE* handle() const
	{
		return file;
	}

private:
	std::FILE* file;
	bool is_process;
};
}

void FileCompare::compare(
	const QString& first,
	const QString& second,
	const QString& lcommand,
	const QString& rcommand,
	long long      filesizelimit // in megabytes
)
{
	FileOrProcess file1(first, lcommand);
	FileOrProcess file2(second, rcommand);

	const bool res = ( pbl::fs::compare(file1.handle(), file2.handle(), filesizelimit * 1024 * 1024) == pbl::fs::compare_equal );

	emit compared(first, second, res);
}
