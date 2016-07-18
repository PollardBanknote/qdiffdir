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
#include "filesystem.h"

#include "cpp/filesystem.h"

namespace
{
bool is_hidden(const cpp::filesystem::path& p)
{
	const std::string s = p.filename().native();

	return !s.empty() && s[0] == '.';
}

}

std::pair< std::vector< std::string >, std::vector< std::string > > FileSystem::contents(const std::string& path)
{
	std::pair< std::vector< std::string >, std::vector< std::string > > res;

	const bool hidden_dirs  = false;
	const bool hidden_files = false;

	for ( cpp::filesystem::directory_iterator it(path), last; it != last; ++it )
	{
		cpp::filesystem::file_status s = it->status();

		if ( cpp::filesystem::is_directory(s))
		{
			if ( hidden_dirs || !is_hidden(it->get_path()))
			{
				const cpp::filesystem::path rel = it->get_path().lexically_relative(path);
				res.second.push_back(rel.native());
			}
		}
		else if ( cpp::filesystem::is_regular_file(s) || cpp::filesystem::is_symlink(s))
		{
			if ( hidden_files || !is_hidden(it->get_path()))
			{
				const cpp::filesystem::path rel = it->get_path().lexically_relative(path);
				res.first.push_back(rel.native());
			}
		}
	}

	return res;
}

bool FileSystem::exists(const std::string& s)
{
	return cpp::filesystem::exists(s);
}

bool FileSystem::copy(
	const std::string& from,
	const std::string& to
)
{
	return cpp::filesystem::copy_file(from, to, copy_options::overwrite_existing);
}

bool FileSystem::is_directory(const std::string& s)
{
	return cpp::filesystem::is_directory(s);
}

bool FileSystem::is_absolute(const std::string& s)
{
	return !s.empty() && s[0] == '/';
}
