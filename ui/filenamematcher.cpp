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
#include "filenamematcher.h"

#include <iostream>

#include <QRegularExpression>
#include <QString>
#include <QDebug>

#include "pbl/util/strings.h"

/// @todo file.bak
FileNameMatcher::FileNameMatcher(const std::vector< FileNameMatcher::match_descriptor >& conditions_)
	: conditions(conditions_)
{

}

FileNameMatcher::match_result FileNameMatcher::compare(
	const std::string& a,
	const std::string& b
) const
{
	// Check one way, then the other
	const match_result res1 = compare_inner(a, b);
	const match_result res2 = compare_inner(b, a);

	if ( res1.weight == -1 )
	{
		return res2;
	}

	if ( res2.weight == -1 )
	{
		return res1;
	}

	return ( res1.weight < res2.weight ) ? res1 : res2;
}

FileNameMatcher::match_result FileNameMatcher::compare_inner(
	const std::string& a,
	const std::string& b
) const
{
	if ( a == b )
	{
		match_result t = { 0, "", "" };
		return t;
	}

	std::size_t best = static_cast< std::size_t >( -1 );

	for ( std::size_t i = 0; i < conditions.size(); ++i )
	{
		QRegularExpression pattern("^" + conditions[i].pattern + "$");
		QString            replace = conditions[i].replacement;

		QString b2 = QString::fromStdString(a);
		b2.replace(pattern, replace);

		if ( b2.toStdString() == b )
		{
			if ( best == static_cast< std::size_t >( -1 ) || conditions[i].weight < conditions[best].weight )
			{
				best = i;
			}
		}
	}

	if ( best != static_cast< std::size_t >( -1 ) )
	{
		match_result t = { conditions[best].weight, conditions[best].first_command.toStdString(), conditions[best].second_command.toStdString() };
		return t;
	}
	else
	{
		match_result fail = { -1, "", "" };
		return fail;
	}
}
