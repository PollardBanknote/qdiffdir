#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <vector>
#include <string>

class FileSystem
{
public:
	// return (files, subdirs) of path
	static std::pair< std::vector< std::string >, std::vector< std::string > > contents(const std::string&);
	static bool exists(const std::string&);
	static bool copy(const std::string&, const std::string&);
	static bool is_directory(const std::string&);
	static bool is_absolute(const std::string&);
private:
};

#endif // FILESYSTEM_H
