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

#include "pbl/fileutil/compare.h"
#include "pbl/fileutil/reduce_paths.h"
#include "pbl/util/strings.h"
#include "pbl/process/which.h"

#include "qutility/icons.h"
#include "qutility/convert.h"

#include "compare.h"
#include "matcher.h"
#include "mysettings.h"
#include "filenamematcher.h"

DirDiffForm::DirDiffForm(QWidget* parent_)
	: QWidget(parent_),
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

	ui->copytoleft->setIcon( get_icon("edit-copy") );
	ui->copytoright->setIcon( get_icon("edit-copy") );
	ui->openleftdir->setIcon( get_icon("folder") );
	ui->openrightdir->setIcon( get_icon("folder") );
	ui->refresh->setIcon( get_icon("view-refresh") );
	ui->openleft->setIcon( get_icon("folder-open") );
	ui->openright->setIcon( get_icon("folder-open") );
	ui->renametoleft->setIcon( get_icon("document-save-as") );
	ui->renametoright->setIcon( get_icon("document-save-as") );
	ui->viewdiff->setIcon( get_icon("zoom-in") );

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
	viewfiles( ui->multilistview->currentRow() );
}

void DirDiffForm::viewfiles(int r)
{
	if ( r >= 0 )
	{
		const std::string s1 = list[r].items[0];
		const std::string s2 = list[r].items[1];

		if ( s1.empty() && s2.empty() )
		{
			QMessageBox::warning(this, "No file selected", "Cannot complete action");

			return;
		}

		MySettings& settings = MySettings::instance();

		if ( !s1.empty() && !s2.empty() )
		{
			QStringList l;
			l << QString::fromStdString(section_tree[0].name() + "/" + s1)
			  << QString::fromStdString(section_tree[1].name() + "/" + s2);

			QString program = settings.getDiffTool();

			if ( program.isEmpty() )
			{
				const char* difftools[] =
				{
					"kompare", "gvimdiff", "meld"
				};

				for ( std::size_t i = 0, n = sizeof( difftools ) / sizeof( difftools[0] ); i < n; ++i )
				{
					const std::string path = pbl::which(difftools[i]);

					if ( !path.empty() )
					{
						program = QString::fromStdString(path);
						break;
					}
				}
			}

			if ( program.isEmpty() )
			{
				QMessageBox::critical(this, "Please configure a difftool", "No difftool has been configured and could not find one in the path.");
			}
			else
			{
				QProcess::startDetached(program, l);
			}
		}
		else
		{
			const std::size_t i = ( s1.empty() ? 1 : 0 );

			cpp::filesystem::path p(section_tree[i].name() + "/" + list[r].items[i]);
			cpp::filesystem::path q = p.parent_path();

			QString program = settings.getEditor();

			if ( program.isEmpty() )
			{
				QDesktopServices::openUrl( QUrl( qt::convert( "file://" + p.native() ) ) );
			}
			else
			{
				QProcess::startDetached( program, QStringList( qt::convert( p.filename().native() ) ), qt::convert( q.native() ) );
			}
		}
	}
}

void DirDiffForm::saveAs(
	std::size_t ifrom,
	std::size_t ito
)
{
	const std::vector< std::string >& filenames   = get_section_files(ifrom);
	const std::string&                source      = section_tree[ifrom].name();
	const std::string&                destination = section_tree[ito].name();

	if ( !source.empty() && !destination.empty() )
	{
		std::set< std::string > changed;

		for ( std::size_t i = 0; i < filenames.size(); ++i )
		{
			const std::string& original_filename = filenames[i];

			QStringList suggested_relative_path = qt::convert(original_filename).split(QDir::separator(), QString::SkipEmptyParts);
			suggested_relative_path.pop_back(); // remove the filename

			QDir suggested_save_directory( qt::convert(destination) );

			bool valid_dir = true;

			for ( int i = 0; valid_dir == true && i < suggested_relative_path.size(); ++i )
			{
				valid_dir = suggested_save_directory.cd(suggested_relative_path[i]);
			}

			if ( !valid_dir )
			{
				suggested_save_directory.cd( qt::convert(destination) );
			}

			QString suggested_filename = suggested_save_directory.absolutePath() + QDir::separator() + qt::convert( cpp::filesystem::basename(original_filename) );
			QString save_file_name     = QFileDialog::getSaveFileName(this, "Save as", suggested_filename);

			if ( !save_file_name.isEmpty() )
			{
				QDir                           src( qt::convert(source) );
				std::pair< bool, overwrite_t > res = copyTo(qt::convert( src.absoluteFilePath( qt::convert(original_filename) ) ), qt::convert(save_file_name), OVERWRITE_ASK);

				if ( res.first )
				{
					changed.insert( qt::convert(save_file_name) );
				}
			}
		}

		if ( !changed.empty() )
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
	const std::string&                from = section_tree[ifrom].name();
	const std::string&                to   = section_tree[ito].name();

	if ( from.empty() || to.empty() )
	{
		return;
	}

	if ( !rels.empty() )
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

		if ( !changed.empty() )
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
		if ( !list[indices[i]].items[j].empty() )
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

	return qt::convert( QFileDialog::getExistingDirectory( this, "Choose a directory", qt::convert(dir) ) );

	#else
	QFileDialog dlg(this, dir, dir);
	dlg.setFileMode(QFileDialog::Directory);
	dlg.setOptions(QFileDialog::ShowDirsOnly);
	int x = dlg.exec();

	if ( x == QDialog::Accepted )
	{
		QStringList l = dlg.selectedFiles();

		if ( !l.isEmpty() )
		{
			return l.at(0);
		}
	}

	return QString();

	#endif // if 1
}

void DirDiffForm::on_openleftdir_clicked()
{
	open_section(0);
}

void DirDiffForm::on_openrightdir_clicked()
{
	open_section(1);
}

void DirDiffForm::on_depth_valueChanged(int)
{
	change_depth();
}

// depth has changed, but dirs are the same
void DirDiffForm::change_depth()
{
	const int d = get_depth();

	section_tree[0].change_depth(d);
	section_tree[1].change_depth(d);
	file_list_changed(d, false);
}

void DirDiffForm::open_section(std::size_t i)
{
	std::string t[2];
	t[i] = getDirectory( section_tree[i].name() );

	if ( !t[i].empty() )
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
	const int  d        = get_depth();
	const bool lchanged = section_tree[0].change_root(left, d);
	const bool rchanged = section_tree[1].change_root(right, d);

	if ( lchanged || rchanged )
	{
		file_list_changed(d, true);
	}
}

void DirDiffForm::find_subdirs(
	QStringList&             subdirs,
	const DirectoryContents& n,
	const std::string&       s,
	int                      depth,
	int                      maxdepth
)
{
	if ( n.valid() )
	{
		if ( depth < maxdepth )
		{
			// don't need to watch dirs at max depth
			subdirs << qt::convert( s + n.name() );

			if ( depth + 1 < maxdepth )
			{
				for ( std::size_t i = 0, m = n.dircount(); i < m; ++i )
				{
					find_subdirs(subdirs, n.subdir(i), s + n.name() + "/", depth + 1, maxdepth);
				}
			}
		}
	}
}

QStringList DirDiffForm::find_subdirs(
	const DirectoryContents& n,
	int                      maxdepth
)
{
	QStringList r;

	find_subdirs(r, n, std::string(), 0, maxdepth);

	return r;
}

void DirDiffForm::file_list_changed(
	int  depth,
	bool rootchanged
)
{
	stopDirectoryWatcher();

	// Update the text of the open directory buttons
	if ( !section_tree[0].valid() )
	{
		ui->openleftdir->setText("Open Left Dir");

		if ( !section_tree[1].valid() )
		{
			ui->openrightdir->setText("Open Right Dir");
		}
		else
		{
			ui->openrightdir->setText( qt::convert( cpp::filesystem::basename( section_tree[1].name() ) ) );
		}
	}
	else if ( !section_tree[1].valid() )
	{
		ui->openleftdir->setText( qt::convert( cpp::filesystem::basename( section_tree[0].name() ) ) );
		ui->openrightdir->setText("Open Right Dir");
	}
	else
	{
		const std::pair< std::string, std::string > p = pbl::fs::reduce_paths( section_tree[0].name(), section_tree[1].name() );

		// Prefer to use just the directory name
		ui->openleftdir->setText( qt::convert(p.first) );
		ui->openrightdir->setText( qt::convert(p.second) );
	}

	const MySettings& settings = MySettings::instance();

	// Rematch files
	FileNameMatcher             name_matcher(settings.getMatchRules());
	std::vector< comparison_t > matched = match_directories(name_matcher, section_tree[0], section_tree[1]);

	if ( !rootchanged )
	{
		// Update "list" to match "matched". Update view as well
		std::size_t i = 0, n = list.size();
		std::size_t j = 0, m = matched.size();

		// Both lists are in sorted order
		while ( i < n && j < m )
		{
			if ( list[i] < matched[j] )
			{
				list.erase(list.begin() + i);
				--n;
				ui->multilistview->removeItem(i);
			}
			else if ( matched[j] < list[i] )
			{
				list.insert(list.begin() + i, matched[j]);
				QStringList labels;
				labels << qt::convert(matched[j].items[0]) << qt::convert(matched[j].items[1]);
				ui->multilistview->insertItem(i, labels);
				++n;
				++j;
				++i;
			}
			else
			{
				++i, ++j;
			}
		}

		while ( i < n )
		{
			list.erase(list.begin() + i);
			--n;
			ui->multilistview->removeItem(i);
		}

		while ( j < m )
		{
			list.insert(list.end(), matched[j]);
			QStringList labels;
			labels << qt::convert(matched[j].items[0]) << qt::convert(matched[j].items[1]);
			ui->multilistview->insertItem(i, labels);
			++j;
			++i;
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

	if ( ui->autoRefresh->isChecked() )
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
	if ( cpp::filesystem::exists(to) )
	{
		switch ( overwrite )
		{
		case OVERWRITE_ASK:
		{
			QMessageBox::StandardButton res = QMessageBox::question(this, qt::convert( cpp::filesystem::basename(to) ) + " already exists", "Do you want to overwrite the destination?", QMessageBox::YesToAll | QMessageBox::NoToAll | QMessageBox::Yes | QMessageBox::No);

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

	return std::make_pair(cpp::filesystem::copy_file(from, to, copy_options::overwrite_existing), overwrite);
}

void DirDiffForm::stopDirectoryWatcher()
{
	watcher->removePaths(watched_dirs);
}

void DirDiffForm::startDirectoryWatcher()
{
	watcher->addPaths(watched_dirs);
}

// File system has notified us of a change in one of our directories
/// @todo If we get a lot of these, performance is terrible
void DirDiffForm::contentsChanged(QString dirname_)
{
	const std::string dirname = cpp::filesystem::cleanpath( qt::convert(dirname_) );

	const int d = get_depth();

	section_tree[0].rescan(dirname, d);
	section_tree[1].rescan(dirname, d);

	file_list_changed(d, false);
}

void DirDiffForm::filesChanged(const std::set< std::string >& files)
{
	for ( std::size_t i = 0; i < list.size(); ++i )
	{
		if ( ( section_tree[0].valid() && !list[i].items[0].empty() && files.count(section_tree[0].name() + "/" + list[i].items[0]) != 0 )
		     || ( section_tree[1].valid() && !list[i].items[1].empty() && files.count(section_tree[1].name() + "/" + list[i].items[1]) != 0 ) )
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
	section_tree[0].change_depth(0);
	section_tree[1].change_depth(0);

	for ( std::size_t i = 0, n = list.size(); i < n; ++i )
	{
		list[i].res = NOT_COMPARED;
	}

	change_depth();
}

int DirDiffForm::get_depth()
{
	if ( ui->depthlimit->isChecked() )
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
	if ( section_tree[i].valid() )
	{
		QDesktopServices::openUrl( QUrl( qt::convert( "file://" + section_tree[i].name() ) ) );
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
		setFilter( v.toRegExp() );
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
	if ( hide_section_only[0] && list[i].has_only(0) )
	{
		hideitem = true;
	}

	// Hide items in the right list that do not have a match in the right
	if ( hide_section_only[1] && list[i].has_only(1) )
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
	if ( !hideitem && !filters.isEmpty() )
	{
		hideitem = true;

		for ( int j = 0; j < filters.count(); ++j )
		{
			if ( filters.at(j).exactMatch( qt::convert(list[i].items[0]) )
			     || filters.at(j).exactMatch( qt::convert(list[i].items[1]) ) )
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

		if ( sel.contains(i) )
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

	if ( new_selection.isEmpty() && !sel.isEmpty() )
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
		if ( section_tree[0].name() + "/" + list[i].items[0] == first && section_tree[1].name() + "/" + list[i].items[1] == second )
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
		f.push_back( QRegExp(l.at(i).trimmed(), Qt::CaseSensitive, QRegExp::Wildcard) );
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

void DirDiffForm::show_only_section(
	std::size_t i,
	bool        checked
)
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

		if ( idx >= 0 && static_cast< unsigned >( idx ) < list.size() )
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

		if ( idx >= 0 && static_cast< unsigned >( idx ) < list.size() )
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
		if ( !hidden(i) )
		{
			// items
			ts << qt::convert(list[i].items[0]) << '\t' << qt::convert(list[i].items[1]) << '\t';

			// result of comparison
			if ( list[i].has_only(0) )
			{
				ts << "Left Only";
			}
			else if ( list[i].has_only(1) )
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
	if ( section_tree[0].valid() && section_tree[1].valid() )
	{
		const std::size_t n = list.size();
		std::size_t       j = n;

		for ( std::size_t i = 0; i < n; ++i )
		{
			if ( !list[i].items[0].empty() && !list[i].items[1].empty() && list[i].res == NOT_COMPARED )
			{
				j = i;

				if ( !hidden(i) )
				{
					break;
				}
			}
		}

		if ( j < n )
		{
			MySettings& settings = MySettings::instance();
			emit        compare_files( qt::convert(section_tree[0].name() + "/" + list[j].items[0]), qt::convert(section_tree[1].name() + "/" + list[j].items[1]), qt::convert(list[j].command[0]), qt::convert(list[j].command[1]), settings.getFileSizeCompareLimit() );
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
		if ( list[i].has_only(j) )
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

void DirDiffForm::on_depthlimit_toggled(bool checked)
{
	ui->depth->setEnabled(checked);
	change_depth();
}

void DirDiffForm::populate_filters()
{
	ui->filter->clear();
	ui->filter->addItem( "All Files", QRegExp(".*") );

	MySettings& settings = MySettings::instance();

	const QMap< QString, QString > f = settings.getFilters();

	for ( QMap< QString, QString >::const_iterator it = f.constBegin(); it != f.constEnd(); ++it )
	{
		ui->filter->addItem( it.key() + " (" + it.value() + ")", it.value() );
	}
}
