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
/**
 * @todo Plugin for special diffs
 */
#include "dirdiffform.h"
#include "ui_dirdiffform.h"

#include <iostream>
#include <set>

#include <QTextStream>
#include <QClipboard>
#include <QMessageBox>
#include <QProcess>
#include <QFileDialog>
#include <QInputDialog>
#include <QFileSystemWatcher>
#include <QDesktopServices>
#include <QUrl>

#include "cpp/filesystem.h"

#include "fileutil/compare.h"
#include "util/strings.h"

#include "modules/filesystem.h"
#include "compare.h"
#include "matcher.h"
#include "mysettings.h"
#include "qutilities/icons.h"
#include "qutilities/convert.h"

DirDiffForm::DirDiffForm(QWidget* parent_) :
	QWidget(parent_),
	ui(new Ui::DirDiffForm),
    hide_section_only(),
    hide_identical_items(false), hide_ignored(false),
	watcher()
{
	ui->setupUi(this);
	populate_filters();

	FileCompare* comparer = new FileCompare;
	comparer->moveToThread(&compare_thread);
	connect(&compare_thread, &QThread::finished, comparer, &QObject::deleteLater);
	connect(this, &DirDiffForm::compare_files, comparer, &FileCompare::compare);
	connect(comparer, &FileCompare::compared, this, &DirDiffForm::items_compared);
	compare_thread.start();

	ui->copytoleft->setIcon(get_icon("edit-copy"));
	ui->copytoright->setIcon(get_icon("edit-copy"));
	ui->openleftdir->setIcon(get_icon("folder"));
	ui->openrightdir->setIcon(get_icon("folder"));
	ui->refresh->setIcon(get_icon("view-refresh"));
	ui->openleft->setIcon(get_icon("folder-open"));
	ui->openright->setIcon(get_icon("folder-open"));
	ui->renametoleft->setIcon(get_icon("document-save-as"));
	ui->renametoright->setIcon(get_icon("document-save-as"));
	ui->viewdiff->setIcon(get_icon("zoom-in"));

	ui->multilistview->addAction(ui->actionIgnore);
	ui->multilistview->addAction(ui->actionCopy_To_Clipboard);
	ui->multilistview->addAction(ui->actionSelect_Different);
	ui->multilistview->addAction(ui->actionSelect_Same);
	ui->multilistview->addAction(ui->actionSelect_Left_Only);
	ui->multilistview->addAction(ui->actionSelect_Right_Only);
	connect(ui->multilistview, &MultiList::itemActivated, this, &DirDiffForm::viewfiles);

    watcher = new QFileSystemWatcher(this);
    connect(watcher, &QFileSystemWatcher::directoryChanged, this, &DirDiffForm::contentsChanged);
}

DirDiffForm::~DirDiffForm()
{
	compare_thread.quit();
	compare_thread.wait();
	delete ui;
}

void DirDiffForm::setFlags(
	bool show_left_only,
	bool show_right_only,
	bool show_identical
)
{
	ui->showleftonly->setChecked(show_left_only);
	ui->showrightonly->setChecked(show_right_only);
	ui->showsame->setChecked(show_identical);
}

void DirDiffForm::settingsChanged()
{
	populate_filters();
}

void DirDiffForm::on_viewdiff_clicked()
{
	viewfiles(ui->multilistview->currentRow());
}

void DirDiffForm::viewfiles(int r)
{
	if ( r >= 0 )
	{
		const std::string s1 = list[r].items[0];
		const std::string s2 = list[r].items[1];

		if ( s1.empty() && s2.empty())
		{
			QMessageBox::warning(this, "No file selected", "Cannot complete action");

			return;
		}

		MySettings& settings = MySettings::instance();

		if ( !s1.empty() && !s2.empty())
		{
			QStringList l;
			l << QString::fromStdString(section_tree[0].name + "/" + s1)
			  << QString::fromStdString(section_tree[1].name + "/" + s2);
			QProcess::startDetached(settings.getDiffTool(), l);
		}
		else
		{
			const std::size_t i = ( s1.empty() ? 1 : 0 );

			cpp::filesystem::path p(section_tree[i].name + "/" + list[r].items[i]);
			cpp::filesystem::path q = p.parent_path();
			QProcess::startDetached(settings.getEditor(), QStringList(qt::convert(p.filename().native())), qt::convert(q.native()));
		}
	}
}

void DirDiffForm::saveAs(
        std::size_t ifrom,
        std::size_t ito
)
{
	const std::vector< std::string >& filenames = get_section_files(ifrom);
	const std::string&                source = section_tree[ifrom].name;
	const std::string&                destination = section_tree[ito].name;

	if ( !source.empty() && !destination.empty())
	{
		std::set< std::string > changed;

		for ( std::size_t i = 0; i < filenames.size(); ++i )
		{
			const std::string& original_filename = filenames[i];

			QStringList suggested_relative_path = qt::convert(original_filename).split(QDir::separator(), QString::SkipEmptyParts);
			suggested_relative_path.pop_back(); // remove the filename

			QDir suggested_save_directory(qt::convert(destination));

			bool valid_dir = true;

			for ( int i = 0; valid_dir == true && i < suggested_relative_path.size(); ++i )
			{
				valid_dir = suggested_save_directory.cd(suggested_relative_path[i]);
			}

			if ( !valid_dir )
			{
				suggested_save_directory.cd(qt::convert(destination));
			}

			QString suggested_filename = suggested_save_directory.absolutePath() + QDir::separator() + qt::convert(cpp::filesystem::basename(original_filename));
			QString save_file_name     = QFileDialog::getSaveFileName(this, "Save as", suggested_filename);

			if ( !save_file_name.isEmpty())
			{
				QDir                           src(qt::convert(source));
				std::pair< bool, overwrite_t > res = copyTo(qt::convert(src.absoluteFilePath(qt::convert(original_filename))), qt::convert(save_file_name), OVERWRITE_ASK);

				if ( res.first )
				{
					changed.insert(qt::convert(save_file_name));
				}
			}
		}

		if ( !changed.empty())
		{
			filesChanged(changed);
		}
	}
}

void DirDiffForm::copyfiles(
        std::size_t ifrom,
        std::size_t ito
)
{
	const std::vector< std::string >& rels = get_section_files(ifrom);
	const std::string&                from = section_tree[ifrom].name;
	const std::string&                to = section_tree[ito].name;

	if ( from.empty() || to.empty())
	{
		return;
	}

	if ( !rels.empty())
	{
		overwrite_t overwrite = OVERWRITE_ASK;

		std::set< std::string > changed;

		for ( std::size_t i = 0; i < rels.size(); ++i )
		{
			const std::string rel = rels[i];

			const std::string              source_file = from + "/" + rel;
			const std::string              dest_file   = to + "/" + rel;
			std::pair< bool, overwrite_t > res         = copyTo(source_file, dest_file, overwrite);

			if ( res.first )
			{
				changed.insert(dest_file);
			}

			overwrite = res.second;
		}

		if ( !changed.empty())
		{
			filesChanged(changed);
		}
	}
	else
	{
		QMessageBox::warning(this, "No file selected", "Cannot complete action");

	}
}

std::vector< std::string > DirDiffForm::get_section_files(std::size_t j)
{
	const QList< int > indices = ui->multilistview->selectedRows();

	std::vector< std::string > rels;

	for ( int i = 0, n = indices.count(); i < n; ++i )
	{
		if ( !list[indices[i]].items[j].empty())
		{
			rels.push_back(list[indices[i]].items[j]);
		}
	}

	return rels;
}

void DirDiffForm::on_copytoright_clicked()
{
	copyfiles(0, 1);
}

void DirDiffForm::on_copytoleft_clicked()
{
	copyfiles(1, 0);
}

void DirDiffForm::on_renametoright_clicked()
{
	saveAs(0, 1);
}

void DirDiffForm::on_renametoleft_clicked()
{
	saveAs(1, 0);
}

std::string DirDiffForm::getDirectory(const std::string& dir)
{
	#if 1

	return qt::convert(QFileDialog::getExistingDirectory(this, "Choose a directory", qt::convert(dir)));

	#else
	QFileDialog dlg(this, dir, dir);
	dlg.setFileMode(QFileDialog::Directory);
	dlg.setOptions(QFileDialog::ShowDirsOnly);
	int x = dlg.exec();

	if ( x == QDialog::Accepted )
	{
		QStringList l = dlg.selectedFiles();

		if ( !l.isEmpty())
		{
			return l.at(0);
		}
	}

	return QString();

	#endif
}

void DirDiffForm::on_openleftdir_clicked()
{
	open_section(0);
}

void DirDiffForm::on_openrightdir_clicked()
{
	open_section(1);
}

void DirDiffForm::on_depth_valueChanged(int d)
{
	change_depth(d);
}

// depth has changed, but dirs are the same
void DirDiffForm::change_depth(int d)
{
	change_depth(section_tree[0], d);
	change_depth(section_tree[1], d);
	file_list_changed(d, false);
}

bool DirDiffForm::change_root(
	dirnode&           n,
	const std::string& dir
)
{
	if ( !dir.empty())
	{
		n.name = ( FileSystem::is_absolute(dir) && FileSystem::is_directory(dir))
		         ? cpp::filesystem::cleanpath(dir)
				 : std::string();
		n.children.clear();
		n.files.clear();

		change_depth(n, get_depth());

		return true;
	}

	return false;
}

void DirDiffForm::open_section(std::size_t i)
{
	std::string t[2];
	t[i] = getDirectory(section_tree[i].name);

	if ( !t[i].empty())
	{
		change_dir(t[0], t[1]);
	}
}

// depth has not changed, but one or both directories have changed
void DirDiffForm::change_dir(
	const std::string& left,
	const std::string& right
)
{
	const bool lchanged = change_root(section_tree[0], left);
	const bool rchanged = change_root(section_tree[1], right);

	if ( lchanged || rchanged )
	{
		file_list_changed(get_depth(), true);
	}
}

void DirDiffForm::change_depth(
	dirnode& n,
	int      d
)
{
	if ( n.name.empty())
	{
		n.children.clear();
		n.files.clear();
	}
	else
	{
		change_depth(n, n.name, 0, d);
	}
}

void DirDiffForm::change_depth(
	dirnode&           n,
	const std::string& current_path,
	int                current_depth,
	int                d
)
{
	if ( current_depth < d )
	{
		if ( n.files.empty() && n.children.empty())
		{
			// Need to populate this directory's contents
			std::pair< std::vector< std::string >, std::vector< std::string > > res = FileSystem::contents(current_path);

			std::sort(res.first.begin(), res.first.end());
			n.files.swap(res.first);

			std::sort(res.second.begin(), res.second.end());
			n.children.resize(res.second.size());

			for ( std::size_t i = 0; i < res.second.size(); ++i )
			{
				n.children[i].name.swap(res.second[i]);
			}
		}

		// Move down a level
		for ( std::size_t i = 0; i < n.children.size(); ++i )
		{
			change_depth(n.children[i], current_path + "/" + n.children[i].name, current_depth + 1, d);
		}
	}
	else
	{
		n.children.clear();
		n.files.clear();
	}
}

void DirDiffForm::rematch_section(
    std::size_t                  j,
    ComparisonList& m,
    const dirnode&               r,
	const std::string&           prefix
)
{
	// Recursively apply to subdirectories
	for ( std::size_t i = 0, n = r.children.size(); i < n; ++i )
	{
		rematch_section(j, m, r.children[i], prefix + r.children[i].name + "/");
	}

	for ( std::size_t i = 0, n = r.files.size(); i < n; ++i )
	{
		comparison_t c = { { std::string(), std::string() }, NOT_COMPARED, false };
		c.items[j] = prefix + r.files[i];
		m.push_back(c);
	}
}

void DirDiffForm::rematch(
    ComparisonList& m,
	const dirnode&               l,
	const dirnode&               r,
	const std::string&           prefix
)
{
	// Recursively apply to subdirectories
	{
		std::size_t       il = 0, ir = 0;
		const std::size_t nl = l.children.size();
		const std::size_t nr = r.children.size();

		for (; il < nl && ir < nr;)
		{
			if ( l.children[il].name == r.children[ir].name )
			{
				rematch(m, l.children[il], r.children[ir], prefix + l.children[il].name + "/");
				++il;
				++ir;
			}
			else
			{
				if ( l.children[il].name < r.children[ir].name )
				{
					rematch_section(0, m, l.children[il], prefix + l.children[il].name + "/");
					++il;
				}
				else
				{
					rematch_section(1, m, r.children[ir], prefix + r.children[ir].name + "/");
					++ir;
				}
			}
		}

		for (; il < nl; ++il )
		{
			rematch_section(0, m, l.children[il], prefix + l.children[il].name + "/");
		}

		for (; ir < nr; ++ir )
		{
			rematch_section(1, m, r.children[ir], prefix + r.children[ir].name + "/");
		}
	}

	// Match files
	ComparisonList matched_files;

	const std::size_t nl = l.files.size();
	const std::size_t nr = r.files.size();
	std::size_t       il = 0;
	std::size_t       ir = 0;

	for (; il < nl && ir < nr;)
	{
		comparison_t c = { std::string(), std::string(), NOT_COMPARED, false };

		if ( l.files[il] == r.files[ir] )
		{
			c.items[0]  = prefix + l.files[il];
			c.items[1] = prefix + r.files[ir];
			++il;
			++ir;
		}
		else
		{
			if ( l.files[il] < r.files[ir] )
			{
				c.items[0] = prefix + l.files[il];
				++il;
			}
			else
			{
				c.items[1] = prefix + r.files[ir];
				++ir;
			}
		}

		matched_files.push_back(c);
	}

	for (; il < nl; ++il )
	{
		comparison_t c = { prefix + l.files[il], std::string(), NOT_COMPARED, false };
		matched_files.push_back(c);
	}

	for (; ir < nr; ++ir )
	{
		comparison_t c = { std::string(), prefix + r.files[ir], NOT_COMPARED, false };
		matched_files.push_back(c);
	}

	// Second pass to match non-exact names
	for ( std::size_t i = 0, n = matched_files.size(); i < n; ++i )
	{
		// Unmatched left item
		if ( matched_files[i].items[1].empty())
		{
			// Find the best match
			std::size_t ibest = 0;
			int         xbest = -1;

			for ( std::size_t j = 0; j < n; ++j )
			{
				if ( matched_files[j].items[0].empty())
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
				matched_files.erase(ibest);
				--n;

				if ( ibest < i )
				{
					--i;
				}
			}
		}
	}

	m.append(matched_files);
}

void DirDiffForm::find_subdirs(
	QStringList&       subdirs,
	const dirnode&     n,
	const std::string& s,
	int                depth,
	int                maxdepth
)
{
	if ( !n.name.empty())
	{
		if ( depth < maxdepth )
		{
			// don't need to watch dirs at max depth
			subdirs << qt::convert(s + n.name);

			if ( depth + 1 < maxdepth )
			{
				for ( std::size_t i = 0; i < n.children.size(); ++i )
				{
					find_subdirs(subdirs, n.children[i], s + n.name + "/", depth + 1, maxdepth);
				}
			}
		}
	}
}

QStringList DirDiffForm::find_subdirs(
	const dirnode& n,
	int            maxdepth
)
{
	QStringList r;

	find_subdirs(r, n, std::string(), 0, maxdepth);

	return r;
}

void replace_intermediate_paths(std::string& s)
{
	const std::size_t i = s.find_first_of('/');

	if ( i != std::string::npos )
	{
		const std::size_t j = s.find_last_of('/');

		if ( j > i )
		{
			s.replace(i + 1, j - i - 1, "..");
		}
	}
}

void DirDiffForm::file_list_changed(
	int  depth,
	bool rootchanged
)
{
	stopDirectoryWatcher();

	// Update the text of the open directory buttons
	if ( section_tree[0].name.empty())
	{
		ui->openleftdir->setText("Open Left Dir");

		if ( section_tree[1].name.empty())
		{
			ui->openrightdir->setText("Open Right Dir");
		}
		else
		{
			ui->openrightdir->setText(qt::convert(cpp::filesystem::basename(section_tree[1].name)));
		}
	}
	else if ( section_tree[1].name.empty())
	{
		ui->openleftdir->setText(qt::convert(cpp::filesystem::basename(section_tree[0].name)));
		ui->openrightdir->setText("Open Right Dir");
	}
	else
	{
		// Prefer to use just the directory name
		std::string l = cpp::filesystem::basename(section_tree[0].name);
		std::string r = cpp::filesystem::basename(section_tree[1].name);

		if ( l == r )
		{
			l = section_tree[0].name;
			r = section_tree[1].name;

			// Find common ancestor
			std::size_t       i = 0;
			const std::size_t n = std::min(l.length(), r.length());

			while ( i < n && l[i] == r[i] )
			{
				++i;
			}

			while ( i > 0 && l[i - 1] != '/' )
			{
				--i;
			}

			// Erase common ancestor
			l.erase(0, i);
			r.erase(0, i);

			// Remove intermediate directories
			replace_intermediate_paths(l);
			replace_intermediate_paths(r);
		}

		ui->openleftdir->setText(qt::convert(l));
		ui->openrightdir->setText(qt::convert(r));
	}

	// Rematch files
	ComparisonList matched;

	if ( !section_tree[0].name.empty() && !section_tree[1].name.empty())
	{
		rematch(matched, section_tree[0], section_tree[1], "");
	}
	else if ( !section_tree[0].name.empty())
	{
		rematch_section(0, matched, section_tree[0], "");
	}
	else if ( !section_tree[1].name.empty())
	{
		rematch_section(1, matched, section_tree[1], "");
	}

	if ( !rootchanged )
	{
		// Reuse the previous information
		for ( std::size_t i = 0, n = list.size(); i < n;)
		{
			std::vector< comparison_t >::iterator it = std::lower_bound(matched.begin(), matched.end(), list[i], compare_by_items);

			if ( it != matched.end() && list[i].items[0] == it->items[0] && list[i].items[1] == it->items[1] )
			{
				// keep item
				matched.erase(it);
				++i;
			}
			else
			{
				// remove item
				list.erase(i);
				--n;
				ui->multilistview->removeItem(i);
			}
		}

		// Insert new items
		for ( std::size_t i = 0, n = matched.size(); i < n; ++i )
		{
			std::vector< comparison_t >::iterator it = std::upper_bound(list.begin(), list.end(), matched[i], compare_by_items);
			const std::size_t                     j  = it - list.begin();
			list.insert(it, matched[i]);
			QStringList labels;
			labels << qt::convert(matched[i].items[0]) << qt::convert(matched[i].items[1]);
			ui->multilistview->insertItem(j, labels);
		}
	}
	else
	{
		ui->multilistview->clear();
		list.swap(matched);

		for ( std::size_t i = 0; i < list.size(); ++i )
		{
			QStringList items;
			items << qt::convert(list[i].items[0]) << qt::convert(list[i].items[1]);
			ui->multilistview->addItem(items);
		}
	}

	applyFilters();

	// Update file system watcher
	watched_dirs.clear();
	watched_dirs << find_subdirs(section_tree[0], depth)
	             << find_subdirs(section_tree[1], depth);
	watched_dirs.removeDuplicates();

	if ( ui->autoRefresh->isChecked())
	{
		startDirectoryWatcher();
	}

	startComparison();
}

void DirDiffForm::changeDirectories(
	const std::string& left,
	const std::string& right
)
{
	change_dir(left, right);
}

std::pair< bool, DirDiffForm::overwrite_t > DirDiffForm::copyTo(
	const std::string& from,
	const std::string& to,
	overwrite_t        overwrite
)
{
	if ( FileSystem::exists(to))
	{
		switch ( overwrite )
		{
		case OVERWRITE_ASK:
		{
			QMessageBox::StandardButton res = QMessageBox::question(this, qt::convert(cpp::filesystem::basename(to)) + " already exists", "Do you want to overwrite the destination?", QMessageBox::YesToAll | QMessageBox::NoToAll | QMessageBox::Yes | QMessageBox::No);

			if ( res == QMessageBox::No )
			{
				return std::make_pair(false, OVERWRITE_ASK);
			}
			else if ( res == QMessageBox::NoToAll )
			{
				return std::make_pair(false, OVERWRITE_NO);
			}
			else if ( res == QMessageBox::YesToAll )
			{
				overwrite = OVERWRITE_YES;
			}
		}
		break;
		case OVERWRITE_YES:
			break;
		case OVERWRITE_NO:

			return std::make_pair(false, OVERWRITE_NO);
		} // switch

	}

	return std::make_pair(FileSystem::copy(from, to), overwrite);
}

void DirDiffForm::stopDirectoryWatcher()
{
    watcher->removePaths(watched_dirs);
}

void DirDiffForm::startDirectoryWatcher()
{
    watcher->addPaths(watched_dirs);
}

// dirname begins with current_path + "/"
bool DirDiffForm::rescan(
	dirnode&           n,
	const std::string& current_path,
	const std::string& dirname,
	int                depth,
	int                maxdepth
)
{
	/// @todo binary search because children is sorted
	for ( std::size_t i = 0; i < n.children.size(); ++i )
	{
		const std::string s = current_path + "/" + n.children[i].name;

		if ( dirname == s )
		{
			// Found the dir
			if ( FileSystem::is_directory(s))
			{
				n.children[i].children.clear();
				n.children[i].files.clear();
				change_depth(n.children[i], s, depth + 1, maxdepth);
			}
			else
			{
				// no longer exists on disk
				n.children.erase(n.children.begin() + i);
			}

			return true;
		}
		else if ( pbl::starts_with(dirname, s + "/"))
		{

			// Descend
			return rescan(n.children[i], s, dirname, depth + 1, maxdepth);
		}
	}

	return false;
}

void DirDiffForm::rescan(
	dirnode&           n,
	const std::string& dirname,
	int                maxdepth
)
{
	if ( !n.name.empty())
	{
		if ( n.name == dirname )
		{
			// root has changed
			n.children.clear();
			n.files.clear();

			if ( FileSystem::is_directory(dirname))
			{
				change_depth(n, maxdepth);
			}
			else
			{
				// directory doesn't exist anymore
				n.name.clear();
			}
		}
		else
		{
			if ( pbl::starts_with(dirname, n.name + "/"))
			{
				rescan(n, n.name, dirname, 0, maxdepth);
			}
		}
	}
}

// File system has notified us of a change in one of our directories
/// @todo If we get a lot of these, performance is terrible
void DirDiffForm::contentsChanged(QString dirname_)
{
	const std::string dirname = cpp::filesystem::cleanpath(qt::convert(dirname_));

	const int d = get_depth();

	rescan(section_tree[0], dirname, d);
	rescan(section_tree[1], dirname, d);

	file_list_changed(d, false);
}

void DirDiffForm::filesChanged(const std::set< std::string >& files)
{
	for ( std::size_t i = 0; i < list.size(); ++i )
	{
		if (( !section_tree[0].name.empty() && !list[i].items[0].empty() && files.count(section_tree[0].name + "/" + list[i].items[0]) != 0 )
		    || ( !section_tree[1].name.empty() && !list[i].items[1].empty() && files.count(section_tree[1].name + "/" + list[i].items[1]) != 0 ))
		{
			list[i].res = NOT_COMPARED;
		}
	}

	startComparison();
}

void DirDiffForm::on_refresh_clicked()
{
	refresh();
}

void DirDiffForm::refresh()
{
	section_tree[0].children.clear();
	section_tree[0].files.clear();
	section_tree[1].children.clear();
	section_tree[1].files.clear();
	change_depth(get_depth());
}

int DirDiffForm::get_depth()
{
	if ( ui->depthlimit->isChecked())
	{
		return ui->depth->value();
	}

	return INT_MAX;
}

void DirDiffForm::on_swap_clicked()
{
	section_tree[0].swap(section_tree[1]);

	for ( std::size_t i = 0; i < list.size(); ++i )
	{
		std::swap(list[i].items[0], list[i].items[1]);
	}

	file_list_changed(get_depth(), true);
}

void DirDiffForm::explore_section(std::size_t i)
{
	if ( !section_tree[i].name.empty())
	{
		QDesktopServices::openUrl(QUrl(qt::convert("file://" + section_tree[i].name)));
	}
}

void DirDiffForm::on_openright_clicked()
{
	explore_section(1);
}

void DirDiffForm::on_openleft_clicked()
{
	explore_section(0);
}

void DirDiffForm::on_showleftonly_toggled(bool checked)
{
	show_only_section(0, checked);
}

void DirDiffForm::on_showrightonly_toggled(bool checked)
{
	show_only_section(1, checked);
}

void DirDiffForm::on_showignored_toggled(bool checked)
{
	showIgnored(checked);
}

void DirDiffForm::on_showsame_toggled(bool checked)
{
	showSame(checked);
}

void DirDiffForm::on_filter_activated(int index)
{
	const QVariant& v = ui->filter->itemData(index);

	if ( v.type() == QVariant::RegExp )
	{
		// A reg ex is stored
		setFilter(v.toRegExp());
	}
	else
	{
		QString s;

		if ( v.type() == QVariant::String )
		{
			s = v.toString();
		}
		else
		{
			s = ui->filter->itemText(index);
		}

		setFilters(s);
	}
}

void DirDiffForm::on_autoRefresh_stateChanged(int state)
{
	if ( state == Qt::Checked )
	{
		startDirectoryWatcher();
	}
	else
	{
		stopDirectoryWatcher();
	}
}

const std::size_t NOT_FOUND = std::size_t(-1);

bool DirDiffForm::hidden(std::size_t i) const
{
	bool hideitem = false;

	// Hide items in the left list that do not have a match in the right
	if ( hide_section_only[0] && list[i].has_only(0))
	{
		hideitem = true;
	}

	// Hide items in the right list that do not have a match in the right
	if ( hide_section_only[1] && list[i].has_only(1))
	{
		hideitem = true;
	}

	// Hide ignored items
	if ( hide_ignored && list[i].ignore )
	{
		hideitem = true;
	}

	// Hide items that have compared identical
	if ( hide_identical_items && list[i].res == COMPARED_SAME )
	{
		hideitem = true;
	}

	// Hide items that don't match the current filter
	if ( !hideitem && !filters.isEmpty())
	{
		hideitem = true;

		for ( int j = 0; j < filters.count(); ++j )
		{
			if ( filters.at(j).exactMatch(qt::convert(list[i].items[0]))
			     || filters.at(j).exactMatch(qt::convert(list[i].items[1])))
			{
				hideitem = false;
				break;
			}
		}
	}

	return hideitem;
}

void DirDiffForm::applyFilters()
{
	// save the current selection
	const QList< int > sel = ui->multilistview->selectedRows();

	QList< int > new_selection;

	bool              seen_selected                   = false;
	const std::size_t n                               = list.size();
	std::size_t       first_unselected_after_selected = n;
	std::size_t       last_unselected                 = n;

	// for each item, check if it is shown or not, and adjust font
	for ( std::size_t i = 0; i < n; ++i )
	{
		const bool hideitem = hidden(i);

		ui->multilistview->style(i, list[i].ignore, list[i].unmatched(), list[i].res != NOT_COMPARED, list[i].res == COMPARED_SAME);
		ui->multilistview->setRowHidden(i, hideitem);

		if ( sel.contains(i))
		{
			seen_selected = true;

			if ( !hideitem )
			{
				new_selection.append(i);
			}
		}
		else
		{
			if ( !hideitem )
			{
				if ( seen_selected && first_unselected_after_selected == n )
				{
					first_unselected_after_selected = i;
				}

				last_unselected = i;
			}
		}
	}

	if ( new_selection.isEmpty() && !sel.isEmpty())
	{
		// select the first visible row after the selection begins
		if ( first_unselected_after_selected != n )
		{
			new_selection << first_unselected_after_selected;
		}
		else if ( last_unselected != n )
		{
			new_selection << last_unselected;
		}
	}

	ui->multilistview->setSelectedRows(new_selection);
}

void DirDiffForm::items_compared(
	const QString& first_,
	const QString& second_,
	bool           equal
)
{

	const std::string first  = qt::convert(first_);
	const std::string second = qt::convert(second_);

	for ( std::size_t i = 0, n = list.size(); i < n; ++i )
	{
		if ( section_tree[0].name + "/" + list[i].items[0] == first && section_tree[1].name + "/" + list[i].items[1] == second )
		{
			list[i].res = equal ? COMPARED_SAME : COMPARED_DIFFERENT;
			applyFilters();
			break;
		}
	}

	startComparison();
}

void DirDiffForm::setFilter(const QRegExp& r)
{
	filters.clear();
	filters.push_back(r);
	applyFilters();
}

void DirDiffForm::setFilters(const QString& s)
{
	QStringList l = s.split(';');

	QVector< QRegExp > f;

	for ( int i = 0; i < l.count(); ++i )
	{
		f.push_back(QRegExp(l.at(i).trimmed(), Qt::CaseSensitive, QRegExp::Wildcard));
	}

	filters = f;
	applyFilters();
}

void DirDiffForm::showIgnored(bool checked)
{
	hide_ignored = !checked;
	applyFilters();
}

void DirDiffForm::showSame(bool checked)
{
	hide_identical_items = !checked;
	applyFilters();
}

void DirDiffForm::show_only_section(std::size_t i, bool checked)
{
	hide_section_only[i] = !checked;
	applyFilters();
}

void DirDiffForm::on_actionIgnore_triggered()
{
	const QList< int > l = ui->multilistview->selectedRows();

	bool some_ignored     = false;
	bool some_not_ignored = false;

	for ( int i = 0, n = l.count(); i < n; ++i )
	{
		const int idx = l.at(i);

		if ( idx >= 0 && static_cast< unsigned >( idx ) < list.size())
		{
			if ( list[idx].ignore )
			{
				some_ignored = true;
			}
			else
			{
				some_not_ignored = true;
			}
		}
	}

	bool all_ignored = some_ignored && !some_not_ignored;

	bool ignore = ( all_ignored ? false : true );

	for ( int i = 0, n = l.count(); i < n; ++i )
	{
		const int idx = l.at(i);

		if ( idx >= 0 && static_cast< unsigned >( idx ) < list.size())
		{
			list[idx].ignore = ignore;
		}
	}

	applyFilters();
}

void DirDiffForm::on_actionCopy_To_Clipboard_triggered()
{
	QClipboard* clipboard = QApplication::clipboard();
	QString     temp;
	QTextStream ts(&temp);

	ts << section_name[0] << "\t" << section_name[1] << endl;

	for ( std::size_t i = 0, n = list.size(); i < n; ++i )
	{
		if ( !hidden(i))
		{
			// items
			ts << qt::convert(list[i].items[0]) << '\t' << qt::convert(list[i].items[1]) << '\t';

			// result of comparison
			if ( list[i].has_only(0))
			{
				ts << "Left Only";
			}
			else if ( list[i].has_only(1))
			{
				ts << "Right Only";
			}
			else if ( list[i].res != NOT_COMPARED )
			{
				if ( list[i].res == COMPARED_SAME )
				{
					ts << "Same";
				}
				else
				{
					ts << "Different";
				}
			}
			else
			{
				ts << "Unknown";
			}

			// ignored or not
			if ( list[i].ignore )
			{
				ts << "\tIgnore";
			}

			ts << endl;
		}
	}

	ts.flush();
	clipboard->setText(temp);
}

void DirDiffForm::startComparison()
{
	if ( !section_tree[0].name.empty() && !section_tree[1].name.empty())
	{
		const std::size_t n = list.size();
		std::size_t       j = n;

		for ( std::size_t i = 0; i < n; ++i )
		{
			if ( !list[i].items[0].empty() && !list[i].items[1].empty() && list[i].res == NOT_COMPARED )
			{
				if ( !hidden(i))
				{
					emit compare_files(qt::convert(section_tree[0].name + "/" + list[i].items[0]), qt::convert(section_tree[1].name + "/" + list[i].items[1]));

					return;
				}

				j = i;
			}
		}

		if ( j < n )
		{
			emit compare_files(qt::convert(section_tree[0].name + "/" + list[j].items[0]), qt::convert(section_tree[1].name + "/" + list[j].items[1]));
		}
	}
}

void DirDiffForm::on_actionSelect_Different_triggered()
{
	QList< int > indices;

	for ( std::size_t i = 0, n = list.size(); i < n; ++i )
	{
		if ( list[i].res == COMPARED_DIFFERENT )
		{
			indices << i;
		}
	}

	ui->multilistview->setSelectedRows(indices);
}

void DirDiffForm::on_actionSelect_Same_triggered()
{
	QList< int > indices;

	for ( std::size_t i = 0, n = list.size(); i < n; ++i )
	{
		if ( list[i].res == COMPARED_SAME )
		{
			indices << i;
		}
	}

	ui->multilistview->setSelectedRows(indices);
}

void DirDiffForm::select_section_only(std::size_t j)
{
	QList< int > indices;

	for ( std::size_t i = 0, n = list.size(); i < n; ++i )
	{
		if ( list[i].has_only(j))
		{
			indices << i;
		}
	}

	ui->multilistview->setSelectedRows(indices);
}

void DirDiffForm::on_actionSelect_Left_Only_triggered()
{
	select_section_only(0);
}

void DirDiffForm::on_actionSelect_Right_Only_triggered()
{
	select_section_only(1);
}

bool DirDiffForm::compare_by_items(
	const DirDiffForm::comparison_t& a,
	const DirDiffForm::comparison_t& b
)
{
	const std::string l  = a.items[0].empty() ? a.items[1] : a.items[0];
	const std::string r  = b.items[0].empty() ? b.items[1] : b.items[0];
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

void DirDiffForm::on_depthlimit_toggled(bool checked)
{
	ui->depth->setEnabled(checked);
	change_depth(get_depth());
}

void DirDiffForm::populate_filters()
{
	ui->filter->clear();
	ui->filter->addItem("All Files", QRegExp(".*"));

	MySettings& settings = MySettings::instance();

	const QMap< QString, QString > f = settings.getFilters();

	for ( QMap< QString, QString >::const_iterator it = f.constBegin(); it != f.constEnd(); ++it )
	{
		ui->filter->addItem(it.key() + " (" + it.value() + ")", it.value());
	}
}
