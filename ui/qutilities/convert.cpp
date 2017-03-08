/* Copyright (c) 2016, Pollard Banknote Limited
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
#include "convert.h"

namespace qt
{
std::string convert(const QString& s)
{
	return s.toStdString();
}

QString convert(const std::string& s)
{
	return QString::fromStdString(s);
}

std::list< std::string > convert(const QStringList& l)
{
	std::list< std::string > r;

	for ( int i = 0; i < l.count(); ++i )
	{
		r.push_back( convert( l.at(i) ) );
	}

	return r;
}

QStringList convert(const std::vector< std::string >& v)
{
	QStringList r;

	for ( std::size_t i = 0; i < v.size(); ++i )
	{
		r << convert(v[i]);
	}

	return r;
}

QStringList convert(const std::list< std::string >& l)
{
	QStringList r;

	for ( std::list< std::string >::const_iterator it = l.begin(), last = l.end(); it != last; ++it )
	{
		r << convert(*it);
	}

	return r;
}

}
