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
#include "comparisonlist.h"

#include <algorithm>

#include "pbl/fileutil/directorycontents.h"

#include "filenamematcher.h"

namespace
{
/* Compare paths directory by directory. Last path component of each must be
 * a file. Files are sorted after directories within the same directory
 */
bool compare_paths(
	const std::string& l,
	const std::string& r
)
{
	const std::size_t nl = l.length();
	const std::size_t nr = r.length();

	// directories before files
	std::size_t il = 0; // start of path component
	std::size_t ir = 0;

	while ( true )
	{
		std::size_t jl = il; // find end of path component
		std::size_t jr = ir;

		while ( jl < nl && l[jl] != '/' )
		{
			++jl;
		}

		while ( jr < nr && r[jr] != '/' )
		{
			++jr;
		}

		if ( jl == nl && jr == nr )
		{

			// at last path component for both
			return l.compare(il, jl - il, r, ir, jr - ir) < 0;
		}

		if ( jl == nl )
		{

			// first points to a file and files come after directories
			return false;
		}

		if ( jr == nr )
		{

			// second points to a file and files come after directories
			return true;
		}

		// both at directories
		const int res = l.compare(il, jl - il, r, ir, jr - ir);

		if ( res != 0 )
		{
			return res < 0;
		}

		// next path component
		il = jl + 1;
		ir = jr + 1;
	}
}

class Rematcher
{
public:
	const std::vector< comparison_t >& rematch(
		const FileNameMatcher&   matcher,
		const DirectoryContents& l,
		const DirectoryContents& r
	)
	{
		rematch(matcher, l, r, "");
		return list;
	}

private:
	void rematch_section(
		const FileNameMatcher&   matcher,
		std::size_t              j,
		const DirectoryContents& r,
		const std::string&       prefix
	)
	{
		// Recursively apply to subdirectories
		for ( std::size_t i = 0, n = r.dircount(); i < n; ++i )
		{
			const DirectoryContents& d = r.subdir(i);
			rematch_section(matcher, j, d, prefix + d.name() + "/");
		}

		for ( std::size_t i = 0, n = r.filecount(); i < n; ++i )
		{
			comparison_t c = { LULZ, { std::string(), std::string() }, { std::string(), std::string() }, NOT_COMPARED, false };
			c.items[j] = prefix + r.filename(i);
			list.push_back(c);
		}

		std::sort( list.begin(), list.end() );
	}

	void rematch(
		const FileNameMatcher&   matcher,
		const DirectoryContents& l,
		const DirectoryContents& r,
		const std::string&       prefix
	)
	{
		if ( l.valid() && r.valid() )
		{
			rematch_inner(matcher, l, r, prefix);
		}
		else if ( l.valid() )
		{
			rematch_section(matcher, 0, l, "");
		}
		else if ( r.valid() )
		{
			rematch_section(matcher, 1, r, "");
		}
	}

	void rematch_inner(
		const FileNameMatcher&   matcher,
		const DirectoryContents& l,
		const DirectoryContents& r,
		const std::string&       prefix
	)
	{
		// Recursively apply to subdirectories
		{
			std::size_t       il = 0, ir = 0;
			const std::size_t nl = l.dircount();
			const std::size_t nr = r.dircount();

			for (; il < nl && ir < nr;)
			{
				const DirectoryContents& ldir = l.subdir(il);
				const DirectoryContents& rdir = r.subdir(ir);

				if ( ldir.name() == rdir.name() )
				{
					rematch(matcher, ldir, rdir, prefix + ldir.name() + "/");
					++il;
					++ir;
				}
				else
				{
					if ( ldir.name() < rdir.name() )
					{
						rematch_section(matcher, 0, ldir, prefix + ldir.name() + "/");
						++il;
					}
					else
					{
						rematch_section(matcher, 1, rdir, prefix + rdir.name() + "/");
						++ir;
					}
				}
			}

			for (; il < nl; ++il )
			{
				const DirectoryContents& ldir = l.subdir(il);
				rematch_section(matcher, 0, ldir, prefix + ldir.name() + "/");
			}

			for (; ir < nr; ++ir )
			{
				const DirectoryContents& rdir = r.subdir(ir);
				rematch_section(matcher, 1, rdir, prefix + rdir.name() + "/");
			}
		}

		// Match files
		std::vector< comparison_t > matched_files;

		const std::size_t nl = l.filecount();
		const std::size_t nr = r.filecount();
		std::size_t       il = 0;
		std::size_t       ir = 0;

		for (; il < nl && ir < nr;)
		{
			comparison_t       c     = { LULZ, { std::string(), std::string() }, { std::string(), std::string() }, NOT_COMPARED, false };
			const std::string& lname = l.filename(il);
			const std::string& rname = r.filename(ir);

			if ( lname == rname )
			{
				c.items[0] = prefix + lname;
				c.items[1] = prefix + rname;
				++il;
				++ir;
			}
			else
			{
				if ( lname < rname )
				{
					c.items[0] = prefix + lname;
					++il;
				}
				else
				{
					c.items[1] = prefix + rname;
					++ir;
				}
			}

			matched_files.push_back(c);
		}

		for (; il < nl; ++il )
		{
			comparison_t c = { LULZ, { prefix + l.filename(il), std::string() }, { std::string(), std::string() }, NOT_COMPARED, false };
			matched_files.push_back(c);
		}

		for (; ir < nr; ++ir )
		{
			comparison_t c = { LULZ, { std::string(), prefix + r.filename(ir) }, { std::string(), std::string() }, NOT_COMPARED, false };
			matched_files.push_back(c);
		}

		// Second pass to match non-exact names
		for ( std::size_t i = 0, n = matched_files.size(); i < n; ++i )
		{
			// Unmatched left item
			if ( matched_files[i].items[1].empty() )
			{
				// Find the best match
				std::size_t ibest = 0;
				int         xbest = -1;

				for ( std::size_t j = 0; j < n; ++j )
				{
					if ( matched_files[j].items[0].empty() )
					{
						const int x = matcher.compare(matched_files[i].items[0], matched_files[j].items[1]);

						if ( x >= 0 )
						{
							if ( xbest == -1 || x < xbest )
							{
								ibest = j;
								xbest = x;
							}
						}
					}
				}

				// If there is one, delete it and fixup i, n
				if ( xbest != -1 )
				{
					matched_files[i].items[1] = matched_files[ibest].items[1];
					matched_files.erase(matched_files.begin() + ibest);
					--n;

					if ( ibest < i )
					{
						--i;
					}
				}
			}
		}

		list.insert( list.end(), matched_files.begin(), matched_files.end() );
		std::sort( list.begin(), list.end() );
	}

	std::vector< comparison_t > list;
};
}

bool comparison_t::has_only(std::size_t i) const
{
	if ( !items[i].empty() )
	{
		for ( std::size_t j = 0; j < 2; ++j )
		{
			if ( j != i && !items[j].empty() )
			{
				return false;
			}
		}

		return true;
	}

	return false;
}

bool comparison_t::unmatched() const
{
	return items[0].empty() || items[1].empty();
}

bool comparison_t::operator<(const comparison_t& b) const
{
	return compare_paths(items[0].empty() ? items[1] : items[0], b.items[0].empty() ? b.items[1] : b.items[0]);
}

std::vector< comparison_t > match_directories(
	const FileNameMatcher&   matcher,
	const DirectoryContents& l,
	const DirectoryContents& r
)
{
	Rematcher t;

	return t.rematch(matcher, l, r);
}
