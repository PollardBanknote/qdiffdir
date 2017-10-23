/* Copyright (c) 2014, Pollard Banknote Limited
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
#include <iostream>
#include <cstdlib>

#include <QApplication>

#include "cpp/filesystem.h"

#include "pbl/process/detach.h"

#include "mainwindow.h"

std::string make_absolute(
	const std::string& s,
	const std::string& cwd
)
{
	if ( !s.empty() )
	{
		if ( s[0] != '/' )
		{
			return cwd + "/" + s;
		}

		return s;
	}

	return std::string();
}

/** @brief Start the application based on command line arguments
 */
int main(
	int   argc,
	char* argv[]
)
{
	const std::string cwd = cpp::filesystem::current_path().native();

	// options for the application
	std::vector< std::string > filenames;

	bool show_left_only  = true;
	bool show_right_only = true;
	bool show_identical  = true;

	// command line processing
	bool no_more_switches = false;
	bool help             = false;

	for ( int i = 1; i < argc; ++i )
	{
		if ( no_more_switches )
		{
			filenames.push_back( make_absolute(argv[i], cwd) );
		}
		else
		{
			std::string s = argv[i];

			if ( s == "--left=show" )
			{
				show_left_only = true;
			}
			else if ( s == "--left=hide" )
			{
				show_left_only = false;
			}
			else if ( s == "--right=show" )
			{
				show_right_only = true;
			}
			else if ( s == "--right=hide" )
			{
				show_right_only = false;
			}
			else if ( s == "--same=show" )
			{
				show_identical = true;
			}
			else if ( s == "--same=hide" )
			{
				show_identical = false;
			}
			else if ( s == "--help" )
			{
				help = true;
			}
			else if ( s == "--" )
			{
				no_more_switches = true;
			}
			else if ( !( s.find("--") == 0 ) )
			{
				filenames.push_back( make_absolute(s, cwd) );
			}
		}
	}

	if ( filenames.size() == 1 )
	{
		filenames.insert(filenames.begin(), cwd);
	}

	if ( help )
	{
		std::cout << "\nUsage: qdiffdir [options] [[leftdir] rightdir]\n\n";
		std::cout << "Options\n\n";
		std::cout << "  --left=[show|hide]  - Show/hide files that are only in the left directory\n";
		std::cout << "                          Default: show\n";
		std::cout << "  --right=[show|hide] - Show/hide files that are only in the right directory\n";
		std::cout << "                          Default: show\n";
		std::cout << "  --same=[show|hide]  - Show/hide identical files\n";
		std::cout << "                          Default: show\n";
		std::cout << "  --filter=source     - Show recognized source files only\n";

		return EXIT_SUCCESS;
	}

	// Detach from the terminal and start up the GUI
	#if 1
	#warning Detach is disabled
	#else
	pbl::process::detach();
	#endif

	QApplication a(argc, argv);

	MainWindow w(filenames, show_left_only, show_right_only, show_identical);
	w.show();

	return a.exec();
}
