#ifndef COMPARISONLIST_H
#define COMPARISONLIST_H

#include <string>
#include <vector>

#include "filenamematcher.h"

struct dirnode
{
	std::string name;
	std::vector< dirnode > children;
	std::vector< std::string > files;

	void swap(dirnode& n)
	{
		name.swap(n.name);
		children.swap(n.children);
		files.swap(n.files);
	}

};

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
			if (!items[i].empty())
			{
				for (std::size_t j = 0; j < 2; ++j)
				{
					if (j != i && !items[j].empty())
						return false;
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

	void rematch(const dirnode&, const dirnode&, const std::string&);

	static bool compare_by_items(const comparison_t& a, const comparison_t& b);
private:
	void rematch_section(std::size_t, const dirnode&, const std::string&);
	void rematch_inner(const dirnode&, const dirnode&, const std::string&);

	FileNameMatcher matcher;

	// maintained in sorted order given by compare_by_items
	std::vector< comparison_t > list;
};


#endif // COMPARISONLIST_H
