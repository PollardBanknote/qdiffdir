/* Copyright (c) 2015, Pollard Banknote Limited
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
#include "mysettings.h"

#include <QSettings>
#include <QString>

const char difftool_key[]      = "difftool";
const char editor_key[]        = "editor";
const char filters_key[]       = "filters";
const char matches_key[]       = "matchrules";
const char compare_limit_key[] = "comparelimit";
const char pattern_key[] = "pattern";
const char replace_key[] = "replace";
const char command1_key[] = "command1";
const char command2_key[] = "command2";
const char weight_key[] = "weight";

MySettings& MySettings::instance()
{
	static MySettings settings;

	return settings;
}

QString MySettings::getEditor() const
{
	return store->value(editor_key).toString();
}

void MySettings::setEditor(const QString& s)
{
	setValue(editor_key, s);
}

QString MySettings::getDiffTool() const
{
	return store->value(difftool_key).toString();
}

void MySettings::setDiffTool(const QString& s)
{
	setValue(difftool_key, s);
}

QMap< QString, QString > MySettings::getFilters() const
{
	const QMap< QString, QVariant > v = store->value(filters_key).toMap();

	QMap< QString, QString > m;

	for ( QMap< QString, QVariant >::const_iterator it = v.constBegin(); it != v.constEnd(); ++it )
	{
		m.insert( it.key(), it.value().toString() );
	}

	return m;
}

void MySettings::setFilters(const QMap< QString, QString >& m)
{
	QMap< QString, QVariant > v;

	for ( QMap< QString, QString >::const_iterator it = m.constBegin(); it != m.constEnd(); ++it )
	{
		v.insert( it.key(), it.value() );
	}

	store->setValue(filters_key, v);
}

int MySettings::getFileSizeCompareLimit() const
{
	QVariant v = store->value(compare_limit_key);

	if ( !v.isNull() )
	{
		return v.toInt();
	}

	return 0;
}

void MySettings::setFileSizeCompareLimit(int x)
{
	store->setValue(compare_limit_key, x);
}

std::vector<FileNameMatcher::match_descriptor> MySettings::getMatchRules() const
{
	std::vector< FileNameMatcher::match_descriptor > v;

	const int n = store->beginReadArray(matches_key);

	for (int i = 0; i < n; ++i)
	{
		store->setArrayIndex(i);

		FileNameMatcher::match_descriptor t =
		{
		    store->value(pattern_key).toString(),
		    store->value(replace_key).toString(),
		    store->value(command1_key).toString(),
		    store->value(command2_key).toString(),
		    store->value(weight_key).toInt()
		};
		v.push_back(t);
	}

	store->endArray();

	return v;
}

void MySettings::setMatchRules(const std::vector<FileNameMatcher::match_descriptor>& v)
{
	const std::size_t n = v.size();
	store->beginWriteArray(matches_key, n);
	for (std::size_t i = 0; i < n; ++i)
	{
		store->setArrayIndex(i);
		store->setValue(pattern_key, v[i].pattern);
		store->setValue(replace_key, v[i].replacement);
		store->setValue(command1_key, v[i].first_command);
		store->setValue(command2_key, v[i].second_command);
		store->setValue(weight_key, v[i].weight);
	}
	store->endArray();
}

MySettings::MySettings()
{
	store = new QSettings("Pollard Banknote", "qdiffdir");
}

MySettings::~MySettings()
{
	delete store;
}

void MySettings::setValue(
	const QString& key,
	const QString& value
)
{
	if ( value.isEmpty() )
	{
		store->remove(key);
	}
	else
	{
		store->setValue(key, value);
	}
}
