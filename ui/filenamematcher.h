#ifndef FILENAMEMATCHER_H
#define FILENAMEMATCHER_H

#include <string>

class FileNameMatcher
{
public:
	int compare(const std::string& a, const std::string& b) const;
private:
	// ext <=> ext.gz
	static std::string gzalt(const std::string& s);

	// c <=> cpp
	static std::string cppalt(const std::string& s);

	// c <=> cpp.gz or cpp <=> c.gz
	static std::string cgalt(const std::string& s);

};


#endif // FILENAMEMATCHER_H
