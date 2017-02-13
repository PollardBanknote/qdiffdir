#ifndef DIRECTORYCONTENTS_H
#define DIRECTORYCONTENTS_H

#include <string>
#include <vector>

class DirectoryContents
{
public:
	void swap(DirectoryContents& n);

	bool valid() const;
	void change_depth();
	void change_depth(int);
	bool change_root(const std::string&, int);
	bool rescan(const std::string&, const std::string&, int, int);
	void rescan(const std::string&, int);
	std::size_t dircount() const;
	std::size_t filecount() const;
	const DirectoryContents& subdir(std::size_t) const;
	const std::string& filename(std::size_t) const;
	const std::string& name() const;
private:
	void init(const std::string&);
	void change_depth(const std::string&, int, int);

	std::string                      name_;
	std::vector< DirectoryContents > children;
	std::vector< std::string >       files;
};


#endif // DIRECTORYCONTENTS_H
