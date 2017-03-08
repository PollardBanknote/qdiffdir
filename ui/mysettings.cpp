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

const char difftool_key[] = "difftool";
const char editor_key[]   = "editor";
const char filters_key[]  = "filters";

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
