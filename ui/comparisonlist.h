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
#ifndef COMPARISONLIST_H
#define COMPARISONLIST_H

#include <string>
#include <vector>

#include "filenamematcher.h"
#include "directorycontents.h"

enum compare_result_t {NOT_COMPARED, COMPARED_SAME, COMPARED_DIFFERENT};

class ComparisonList
{
public:

	struct comparison_t
	{
		std::string items[2]; // { left, right }
		compare_result_t res;
		bool ignore;

		bool has_only(std::size_t i) const
		{
			if ( !items[i].empty())
			{
				for ( std::size_t j = 0; j < 2; ++j )
				{
					if ( j != i && !items[j].empty())
					{
						return false;
					}
				}

				return true;
			}

			return false;
		}

		bool unmatched() const
		{
			return items[0].empty() || items[1].empty();
		}

	};

	typedef std::vector< comparison_t >::iterator iterator;
	typedef std::vector< comparison_t >::const_iterator const_iterator;

	comparison_t& operator[](std::size_t i)
	{
		return list[i];
	}

	const comparison_t& operator[](std::size_t i) const
	{
		return list[i];
	}

	std::size_t size() const
	{
		return list.size();
	}

	void erase(std::size_t i)
	{
		list.erase(list.begin() + i);
	}

	std::pair< iterator, bool > insert(const comparison_t& x);

	void swap(ComparisonList& o)
	{
		list.swap(o.list);
	}

	void rematch(const DirectoryContents&, const DirectoryContents&, const std::string&);

	static bool compare_by_items(const comparison_t& a, const comparison_t& b);

	void forget();
private:
	void rematch_section(std::size_t, const DirectoryContents&, const std::string&);
	void rematch_inner(const DirectoryContents&, const DirectoryContents&, const std::string&);

	FileNameMatcher matcher;

	// maintained in sorted order given by compare_by_items
	std::vector< comparison_t > list;
};


#endif // COMPARISONLIST_H
