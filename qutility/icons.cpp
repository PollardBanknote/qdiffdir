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
#include "icons.h"

#include <QIcon>

QIcon get_icon(const QString& name)
{
	// hopefully we have the free desktop icon
	if ( QIcon::hasThemeIcon(name))
	{
		return QIcon::fromTheme(name);
	}

	// Try using old icon names (if we're on KDE3)
	if ( name == "view-refresh" )
	{
		return QIcon::fromTheme("reload");
	}

	if ( name == "folder-open" )
	{
		return QIcon::fromTheme("folder_open");
	}

	if ( name == "zoom-in" )
	{
		return QIcon::fromTheme("viewmag");
	}

	if ( name == "list-add" )
	{
		return QIcon::fromTheme("add");
	}

	if ( name == "list-remove" )
	{
		return QIcon::fromTheme("remove");
	}

	if ( name == "document-save-as" )
	{
		return QIcon::fromTheme("filesaveas");
	}

	if ( name == "go-down" )
	{
		return QIcon::fromTheme("down");
	}

	return QIcon();
}
