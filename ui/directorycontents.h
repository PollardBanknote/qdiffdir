#ifndef DIRECTORYCONTENTS_H
#define DIRECTORYCONTENTS_H

#include <string>
#include <vector>

struct DirectoryContents
{
	std::string name;
	std::vector< DirectoryContents > children;
	std::vector< std::string > files;

	void swap(DirectoryContents& n);

	void
init(const std::string&);
};


#endif // DIRECTORYCONTENTS_H
