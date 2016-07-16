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

#include "util/file.h"
#include "util/strings.h"
#include "util/containers.h"

#include "compare.h"
#include "matcher.h"
#include "mysettings.h"
#include "qutilities/icons.h"
#include "qutilities/convert.h"

QString lastPathComponent(const QString& s)
{
	return qt::convert(cpp::filesystem::basename(qt::convert(s)));
}

DirDiffForm::DirDiffForm(QWidget* parent_) :
	QWidget(parent_),
	ui(new Ui::DirDiffForm), filter(),
	hide_left_only(false),
	hide_right_only(false), hide_identical_items(false), hide_ignored(false),
	watcher()
{
	ui->setupUi(this);

	FileCompare* comparer = new FileCompare;
	comparer->moveToThread(&compare_thread);
	connect(&compare_thread, SIGNAL(finished()), comparer, SLOT(deleteLater()));
	connect(this, SIGNAL(compare_files(QString,QString)), comparer, SLOT(compare(QString,QString)));
	connect(comparer, SIGNAL(compared(QString,QString,bool)), SLOT(items_compared(QString,QString,bool)));
	compare_thread.start();

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
	connect(ui->multilistview, SIGNAL(itemActivated(int)), SLOT(viewfiles(int)));

	startDirectoryWatcher();
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

void DirDiffForm::on_viewdiff_clicked()
{
	const int r = ui->multilistview->currentRow();

	if ( r >= 0 )
	{
		const std::string s1 = list[r].items.left;
		const std::string s2 = list[r].items.right;

		if ( s1.empty() && s2.empty())
		{
			QMessageBox::warning(this, "No file selected", "Cannot complete action");

			return;
		}

		MySettings& settings = MySettings::instance();

		if ( s1.empty()) // view s2
		{
			cpp::filesystem::path p(rtree.name + "/" + s2);
			cpp::filesystem::path q = p.parent_path();
			QProcess::startDetached(settings.getEditor(), QStringList(qt::convert(p.filename().native())), qt::convert(q.native()));
		}
		else if ( s2.empty()) // view s1
		{
			cpp::filesystem::path p(ltree.name + "/" + s1);
			cpp::filesystem::path q = p.parent_path();
			QProcess::startDetached(settings.getEditor(), QStringList(qt::convert(p.filename().native())), qt::convert(q.native()));
		}
		else
		{
			QStringList l;
			l << QString::fromStdString(ltree.name + "/" + s1)
			  << QString::fromStdString(rtree.name + "/" + s2);
			QProcess::startDetached(settings.getDiffTool(), l);
		}
	}
}

void DirDiffForm::saveAs(
	const std::vector< std::string >& filenames,
	const std::string&                source,
	const std::string&                dest
)
{
	for ( std::size_t i = 0; i < filenames.size(); ++i )
	{
		saveAs(filenames[i], source, dest);
	}
}

void DirDiffForm::saveAs(
	const std::string& original_filename, // a filename relative to source
	const std::string& source,
	const std::string& destination
)
{
	if ( !original_filename.empty())
	{
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
			QString s = lastPathComponent(save_file_name);
			QString t = save_file_name;
			t.chop(s.length());
			QDir                           src(qt::convert(source));
			std::pair< bool, overwrite_t > res = copyTo(qt::convert(src.absoluteFilePath(qt::convert(original_filename))), qt::convert(t), qt::convert(s), OVERWRITE_ASK);

			if ( res.first )
			{
				fileChanged(qt::convert(save_file_name));
			}
		}
	}
}

void DirDiffForm::on_copytoright_clicked()
{
	if ( ltree.name.empty() || rtree.name.empty())
	{
		return;
	}

	const QList< int > indices = ui->multilistview->selectedRows();

	bool nonempty = false;

	overwrite_t overwrite = OVERWRITE_ASK;

    std::set< std::string > changed;

	for ( int i = 0, n = indices.count(); i < n; ++i )
	{
		const std::string rel = list[indices[i]].items.left;

		if ( !rel.empty())
		{
			const std::string              source_file = ltree.name + "/" + rel;
			const std::string              dest_file   = rtree.name + "/" + rel;
			const std::string              dest_dir    = cpp::filesystem::dirname(dest_file);
			const std::string              file_name   = cpp::filesystem::basename(source_file);
			std::pair< bool, overwrite_t > res         = copyTo(source_file, dest_dir, file_name, overwrite);

			if ( res.first )
			{
                changed.insert(dest_file);
			}

			overwrite = res.second;
			nonempty  = true;
		}
	}

	if ( !nonempty )
	{
		QMessageBox::warning(this, "No file selected", "Cannot complete action");

		return;
	}

	if ( !changed.empty())
	{
		filesChanged(changed);
	}
}

void DirDiffForm::on_copytoleft_clicked()
{
	if ( ltree.name.empty() || rtree.name.empty())
	{
		return;
	}

	const QList< int > indices = ui->multilistview->selectedRows();

	bool nonempty = false;

	overwrite_t overwrite = OVERWRITE_ASK;

    std::set< std::string > changed;

	for ( int i = 0, n = indices.count(); i < n; ++i )
	{
		const std::string rel = list[indices[i]].items.right;

		if ( !rel.empty())
		{
			std::string                    source_file = rtree.name + "/" + rel;
			std::string                    dest_file   = ltree.name + "/" + rel;
			std::string                    dest_dir    = cpp::filesystem::dirname(dest_file);
			std::string                    file_name   = cpp::filesystem::basename(source_file);
			std::pair< bool, overwrite_t > res         = copyTo(source_file, dest_dir, file_name, overwrite);

			if ( res.first )
			{
                changed.insert(dest_file);
			}

			overwrite = res.second;
			nonempty  = true;
		}
	}

	if ( !nonempty )
	{
		QMessageBox::warning(this, "No file selected", "Cannot complete action");

		return;
	}

	if ( !changed.empty())
	{
		filesChanged(changed);
	}
}

void DirDiffForm::on_renametoright_clicked()
{
	if ( ltree.name.empty() || rtree.name.empty())
	{
		return;
	}

	const QList< int > indices = ui->multilistview->selectedRows();

	std::vector< std::string > files;

	for ( int i = 0, n = indices.count(); i < n; ++i )
	{
		if ( !list[indices.at(i)].items.left.empty())
		{
			files.push_back(list[indices.at(i)].items.left);
		}
	}

	saveAs(files, ltree.name, rtree.name);
}

void DirDiffForm::on_renametoleft_clicked()
{
	if ( ltree.name.empty() || rtree.name.empty())
	{
		return;
	}

	const QList< int > indices = ui->multilistview->selectedRows();

	std::vector< std::string > files;

	for ( int i = 0, n = indices.count(); i < n; ++i )
	{
		if ( !list[indices.at(i)].items.right.empty())
		{
			files.push_back(list[indices.at(i)].items.right);
		}
	}

	saveAs(files, rtree.name, ltree.name);
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
	const std::string s = getDirectory(ltree.name);

	if ( !s.empty())
	{
		changeDirectories(s, std::string());
	}
}

void DirDiffForm::on_openrightdir_clicked()
{
	const std::string s = getDirectory(rtree.name);

	if ( !s.empty())
	{
		changeDirectories(std::string(), s);
	}
}

void DirDiffForm::viewfiles(int x_)
{
	if ( x_ >= 0 )
	{
		const std::string s1 = list[x_].items.left;
		const std::string s2 = list[x_].items.right;

		MySettings& settings = MySettings::instance();

		if ( s1.empty())
		{
			if ( !s2.empty())
			{
				QProcess::startDetached(settings.getEditor(),
					QStringList(qt::convert(rtree.name + "/" + s2)));
			}
		}
		else if ( s2.empty())
		{
			QProcess::startDetached(settings.getEditor(),
				QStringList(qt::convert(ltree.name + "/" + s1)));
		}
		else
		{
			QStringList l;
			l << qt::convert(ltree.name + "/" + s1)
			  << qt::convert(rtree.name + "/" + s2);
			QProcess::startDetached(settings.getDiffTool(), l);
		}
	}
}

QString DirDiffForm::renumber(const QString& s_)
{
	QString s = s_;
	QRegExp r(".*([0-9]+).*");

	if ( r.exactMatch(s))
	{
		QString t  = r.cap(1);
		bool    ok = false;
		int     x_ = t.toInt();
		int     n  = QInputDialog::getInt(this, "Renumber", "Please enter the new file number", x_, 1, INT_MAX, 1, &ok);

		if ( !ok )
		{
			return QString();
		}

		s.replace(t, QString::number(n));
	}

	return s;
}

void DirDiffForm::on_depth_valueChanged(int d)
{
	change_depth(d);
}

// depth has changed, but dirs are the same
void DirDiffForm::change_depth(int d)
{
	change_depth(ltree, d);
	change_depth(rtree, d);
	file_list_changed(d, false);
}

// depth has not changed, but one or both directories have changed
void DirDiffForm::change_depth(
	int  d,
	bool l,
	bool r
)
{
	if ( l )
	{
		change_depth(ltree, d);
	}

	if ( r )
	{
		change_depth(rtree, d);
	}

	file_list_changed(d, l || r);
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
			std::pair< std::vector< std::string >, std::vector< std::string > > res = scanner.contents(current_path);

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

void DirDiffForm::rematch_left(
	std::vector< comparison_t >& m,
	const dirnode&               l,
	const std::string&           prefix
)
{
	// Recursively apply to subdirectories
	for ( std::size_t i = 0, n = l.children.size(); i < n; ++i )
	{
		rematch_left(m, l.children[i], prefix + l.children[i].name + "/");
	}

	for ( std::size_t i = 0, n = l.files.size(); i < n; ++i )
	{
		comparison_t c = { items_t(prefix + l.files[i], std::string()), NOT_COMPARED, false };
		m.push_back(c);
	}
}

void DirDiffForm::rematch_right(
	std::vector< comparison_t >& m,
	const dirnode&               r,
	const std::string&           prefix
)
{
	// Recursively apply to subdirectories
	for ( std::size_t i = 0, n = r.children.size(); i < n; ++i )
	{
		rematch_right(m, r.children[i], prefix + r.children[i].name + "/");
	}

	for ( std::size_t i = 0, n = r.files.size(); i < n; ++i )
	{
		comparison_t c = { items_t(std::string(), prefix + r.files[i]), NOT_COMPARED, false };
		m.push_back(c);
	}
}

void DirDiffForm::rematch(
	std::vector< comparison_t >& m,
	const dirnode&               l,
	const dirnode&               r,
	const std::string&           prefix
)
{
	// Recursively apply to subdirectories
    {
        std::size_t il = 0, ir = 0;
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
                    rematch_left(m, l.children[il], prefix + l.children[il].name + "/");
                    ++il;
                }
                else
                {
                    rematch_right(m, r.children[ir], prefix + r.children[ir].name + "/");
                    ++ir;
                }
            }
        }

        for (; il < nl; ++il)
        {
            rematch_left(m, l.children[il], prefix + l.children[il].name + "/");
        }

        for (; ir < nr; ++ir)
        {
            rematch_right(m, r.children[ir], prefix + r.children[ir].name + "/");
        }
    }

	// Match files
    std::vector< comparison_t > matched_files;

	const std::size_t nl = l.files.size();
	const std::size_t nr = r.files.size();
	std::size_t       il = 0;
	std::size_t       ir = 0;

	for (; il < nl && ir < nr;)
	{
		comparison_t c = { items_t(std::string(), std::string()), NOT_COMPARED, false };

		if ( l.files[il] == r.files[ir] )
		{
			c.items.left  = prefix + l.files[il];
			c.items.right = prefix + r.files[ir];
			++il;
			++ir;
		}
		else
		{
			if ( l.files[il] < r.files[ir] )
			{
				c.items.left = prefix + l.files[il];
				++il;
			}
			else
			{
				c.items.right = prefix + r.files[ir];
				++ir;
			}
		}

        matched_files.push_back(c);
	}

	for (; il < nl; ++il )
	{
		comparison_t c = { items_t(prefix + l.files[il], std::string()), NOT_COMPARED, false };
        matched_files.push_back(c);
	}

	for (; ir < nr; ++ir )
	{
		comparison_t c = { items_t(std::string(), prefix + r.files[ir]), NOT_COMPARED, false };
        matched_files.push_back(c);
	}

    // Second pass to match non-exact names
    for (std::size_t i = 0, n = matched_files.size(); i < n; ++i)
    {
        // Unmatched left item
        if (matched_files[i].items.right.empty())
        {
            // Find the best match
            std::size_t ibest = 0;
            int xbest = -1;

            for (std::size_t j = 0; j < n; ++j)
                if (matched_files[j].items.left.empty())
                {
                    const int x = matcher.compare(matched_files[i].items.left, matched_files[j].items.right);
                    if (x >= 0)
                    {
                        if (xbest == -1 || x < xbest)
                        {
                            ibest = j;
                            xbest = x;
                        }
                    }
                }

            // If there is one, delete it and fixup i, n
            if (xbest != -1)
            {
                matched_files[i].items.right = matched_files[ibest].items.right;
                matched_files.erase(matched_files.begin() + ibest);
                --n;
                if (ibest < i)
                    --i;
            }
        }
    }

    m.insert(m.end(), matched_files.begin(), matched_files.end());
}

void DirDiffForm::find_subdirs(
	std::set< std::string >& subdirs,
	const dirnode&           n,
	const std::string&       s,
	int                      depth,
	int                      maxdepth
)
{
	if ( !n.name.empty())
	{
		if ( depth < maxdepth )
		{
			// don't need to watch dirs at max depth
			subdirs.insert(s + n.name);

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

void DirDiffForm::file_list_changed(
	int  depth,
	bool rootchanged
)
{
	// Rematch files
	std::vector< comparison_t > matched;

	if ( !ltree.name.empty() && !rtree.name.empty())
	{
		rematch(matched, ltree, rtree, "");
	}
	else if ( !ltree.name.empty())
	{
        rematch_left(matched, ltree, "");
	}
    else if ( !rtree.name.empty())
	{
        rematch_right(matched, rtree, "");
	}

    if (!rootchanged)
    {
        // Reuse the previous information
        for (std::size_t i = 0, n = list.size(); i < n; )
        {
            std::vector< comparison_t >::iterator it = std::lower_bound(matched.begin(), matched.end(), list[i], compare_by_items);
            if (it != matched.end() && list[i].items == it->items)
            {
                // keep item
                matched.erase(it);
                ++i;
            }
            else
            {
                // remove item
                list.erase(list.begin() + i);
                --n;
                ui->multilistview->removeItem(i);
            }
        }

        // Insert new items
        for (std::size_t i = 0, n = matched.size(); i < n; ++i)
        {
            std::vector< comparison_t >::iterator it = std::upper_bound(list.begin(), list.end(), matched[i], compare_by_items);
            const std::size_t j = it - list.begin();
            list.insert(it, matched[i]);
            QStringList labels;
            labels << qt::convert(matched[i].items.left) << qt::convert(matched[i].items.right);
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
            items << qt::convert(list[i].items.left) << qt::convert(list[i].items.right);
            ui->multilistview->addItem(items);
        }
    }

    applyFilters();

	// Update file system watcher
	std::set< std::string > subdirs;
	find_subdirs(subdirs, ltree, std::string(), 0, depth);
	find_subdirs(subdirs, rtree, std::string(), 0, depth);

	std::set< std::string >::iterator first1 = subdirs.begin(), last1 = subdirs.end(), first2 = watched_dirs.begin(), last2 = watched_dirs.end();

	while ( first1 != last1 && first2 != last2 )
	{
		if ( *first1 < *first2 )
		{
			// Adding directory to watch
			if ( watcher )
			{
				watcher->addPath(qt::convert(*first1));
			}

			++first1;
		}
		else if ( *first2 < *first1 )
		{
			// no longer watching path
			if ( watcher )
			{
				watcher->removePath(qt::convert(*first2));
			}

			++first2;
		}
		else
		{
			++first1;
			++first2;
		}
	}

	while ( first1 != last1 )
	{
		if ( watcher )
		{
			watcher->addPath(qt::convert(*first1));
		}

		++first1;
	}

	while ( first2 != last2 )
	{
		if ( watcher )
		{
			watcher->removePath(qt::convert(*first2));
		}

		++first2;
	}

	watched_dirs.swap(subdirs);

	startComparison();
}

void DirDiffForm::changeDirectories(
	const std::string& left,
	const std::string& right
)
{
	const bool lchanged = !left.empty();
	const bool rchanged = !right.empty();

	if ( !left.empty())
	{
		// change to the new directories
		const cpp::filesystem::path lpath(left);

		if ( lpath.is_absolute() && cpp::filesystem::is_directory(lpath))
		{
			ltree.name = cpp::filesystem::cleanpath(lpath);
			ltree.children.clear();
			ltree.files.clear();
			ui->openleftdir->setText(qt::convert(cpp::filesystem::basename(left)));
		}
		else
		{
			// Error
			ltree.name.clear();
			ltree.children.clear();
			ltree.files.clear();
			ui->openleftdir->setText("Open Left Dir");
		}
	}

	if ( !right.empty())
	{
		const cpp::filesystem::path rpath(right);

		if ( rpath.is_absolute() && cpp::filesystem::is_directory(rpath))
		{
			rtree.name = cpp::filesystem::cleanpath(rpath);
			rtree.children.clear();
			rtree.files.clear();
			ui->openrightdir->setText(qt::convert(cpp::filesystem::basename(right)));
		}
		else
		{
			rtree.name.clear();
			rtree.children.clear();
			rtree.files.clear();
			ui->openrightdir->setText("Open Right Dir");
		}
	}

	if ( lchanged || rchanged )
	{
		change_depth(ui->depth->value(), lchanged, rchanged);
	}
}

std::pair< bool, DirDiffForm::overwrite_t > DirDiffForm::copyTo(
	const std::string& file,
	const std::string& destdir,
	const std::string& newname,
	overwrite_t        overwrite
)
{
	const std::string s = destdir + "/" + newname;

	if ( cpp::filesystem::exists(s))
	{
		switch ( overwrite )
		{
		case OVERWRITE_ASK:
		{
			QMessageBox::StandardButton res = QMessageBox::question(this, qt::convert(newname) + " already exists", "Do you want to overwrite the destination?", QMessageBox::YesToAll | QMessageBox::NoToAll | QMessageBox::Yes | QMessageBox::No);

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

	if ( cpp::filesystem::copy_file(file, s, copy_options::overwrite_existing))
	{
		return std::make_pair(true, overwrite);
	}
	else
	{
		return std::make_pair(false, overwrite);
	}
}

void DirDiffForm::stopDirectoryWatcher()
{
	if ( watcher != NULL )
	{
		delete watcher;
		watcher = NULL;
	}
}

void DirDiffForm::startDirectoryWatcher()
{
	if ( !watcher )
	{
		// create new file system watcher
		watcher = new QFileSystemWatcher(this);
		connect(watcher, SIGNAL(directoryChanged(QString)), SLOT(contentsChanged(QString)));

		for ( std::set< std::string >::iterator it = watched_dirs.begin(); it != watched_dirs.end(); ++it )
		{
			watcher->addPath(qt::convert(*it));
		}
	}
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
			if ( cpp::filesystem::is_directory(s))
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

bool DirDiffForm::rescan(
	dirnode&           n,
	const std::string& dirname,
	int                maxdepth
)
{
	if ( n.name.empty())
	{
		return false;
	}

	if ( n.name == dirname )
	{
		// root has changed
		n.children.clear();
		n.files.clear();

		if ( cpp::filesystem::is_directory(dirname))
		{
			change_depth(n, maxdepth);

			return true;
		}
		else
		{
			// directory doesn't exist anymore
			n.name.clear();

			return false;
		}
	}
	else
	{
		if ( pbl::starts_with(dirname, n.name + "/"))
		{
			rescan(n, n.name, dirname, 0, maxdepth);
		}

		return true;
	}
}

// File system has notified us of a change in one of our directories
void DirDiffForm::contentsChanged(QString dirname_)
{
	const std::string dirname = cpp::filesystem::cleanpath(qt::convert(dirname_));

	const int d = ui->depth->value();

	if ( !rescan(ltree, dirname, d))
	{
		ui->openleftdir->setText("Open Left Dir");
	}

	if ( !rescan(rtree, dirname, d))
	{
		ui->openrightdir->setText("Open Right Dir");
	}

	file_list_changed(d, false);
}

// A single file has been changed/or added. Everything else stayed the same
// file is an absolute path
/// @bug Not tracking file changes, but it seems to work anyway
void DirDiffForm::fileChanged(const std::string& file)
{
	if ( !ltree.name.empty() && pbl::starts_with(file, ltree.name + "/"))
	{
		for ( std::size_t i = 0; i < list.size(); ++i )
		{
			if ( file.compare(ltree.name.length() + 1, std::string::npos, list[i].items.left) == 0 )
			{
				list[i].res = NOT_COMPARED;
				break;
			}
		}
	}

	if ( !rtree.name.empty() && pbl::starts_with(file, rtree.name + "/"))
	{
		for ( std::size_t i = 0; i < list.size(); ++i )
		{
			if ( file.compare(rtree.name.length() + 1, std::string::npos, list[i].items.right) == 0 )
			{
				list[i].res = NOT_COMPARED;
				break;
			}
		}
	}
}

void DirDiffForm::filesChanged(const std::set< std::string >& files)
{
    for (std::size_t i = 0; i < list.size(); ++i)
    {
        if ((!ltree.name.empty() && !list[i].items.left.empty() && files.count(ltree.name + "/" + list[i].items.left) != 0)
                || (!rtree.name.empty() && !list[i].items.right.empty() && files.count(rtree.name + "/" + list[i].items.right) != 0))
            list[i].res = NOT_COMPARED;
    }

    startComparison();
}

void DirDiffForm::on_refresh_clicked()
{
	refresh();
}

void DirDiffForm::refresh()
{
	ltree.children.clear();
	ltree.files.clear();
	rtree.children.clear();
	rtree.files.clear();
	change_depth(ui->depth->value());
}

void DirDiffForm::on_swap_clicked()
{
	ltree.swap(rtree);

	for ( std::size_t i = 0; i < list.size(); ++i )
	{
		std::swap(list[i].items.left, list[i].items.right);
	}

	file_list_changed(ui->depth->value(), true);

	QString s = ui->openleftdir->text();
	ui->openleftdir->setText(ui->openrightdir->text());
	ui->openrightdir->setText(s);
}

void DirDiffForm::on_openright_clicked()
{
	if ( !rtree.name.empty())
	{
		QDesktopServices::openUrl(QUrl(qt::convert("file://" + rtree.name)));
	}
}

void DirDiffForm::on_openleft_clicked()
{
	if ( !ltree.name.empty())
	{
		QDesktopServices::openUrl(QUrl(qt::convert("file://" + ltree.name)));
	}
}

void DirDiffForm::on_showleftonly_toggled(bool checked)
{
	showOnlyLeft(checked);
}

void DirDiffForm::on_showrightonly_toggled(bool checked)
{
	showOnlyRight(checked);
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
	switch ( index )
	{
	case 0:
		clearFilter();
		break;
	case 1:
		setFilter(QRegExp(".*(cpp|h)"));
		break;
	}
}

void DirDiffForm::on_filter_editTextChanged(const QString& arg1)
{
	QRegExp rx(arg1);

	setFilter(rx);
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
	if ( hide_left_only && list[i].left_only())
	{
		hideitem = true;
	}

	// Hide items in the right list that do not have a match in the right
	if ( hide_right_only && list[i].right_only())
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
	if ( !filter.isEmpty() && !filter.exactMatch(qt::convert(list[i].items.left)) && !filter.exactMatch(qt::convert(list[i].items.right)))
	{
		hideitem = true;
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
        if ( ltree.name + "/" + list[i].items.left == first && rtree.name + "/" + list[i].items.right == second )
		{
			list[i].res = equal ? COMPARED_SAME : COMPARED_DIFFERENT;
			applyFilters();
            break;
		}
	}

    startComparison();
}

void DirDiffForm::clearFilter()
{
	setFilter(QRegExp());
}

void DirDiffForm::setFilter(const QRegExp& r)
{
	filter = r;
	applyFilters();
}

void DirDiffForm::showOnlyLeft(bool checked)
{
	hide_left_only = !checked;
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

void DirDiffForm::showOnlyRight(bool checked)
{
	hide_right_only = !checked;
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

	ts << leftname << "\t" << rightname << endl;

	for ( std::size_t i = 0, n = list.size(); i < n; ++i )
	{
		if ( !hidden(i))
		{
			// items
			ts << qt::convert(list[i].items.left) << '\t' << qt::convert(list[i].items.right) << '\t';

			// result of comparison
			if ( list[i].left_only())
			{
				ts << "Left Only";
			}
			else if ( list[i].right_only())
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
    if (!ltree.name.empty() && !rtree.name.empty())
    {
        const std::size_t n = list.size();
        std::size_t j = n;

        for ( std::size_t i = 0; i < n; ++i )
        {
            if ( !list[i].items.left.empty() && !list[i].items.right.empty() && list[i].res == NOT_COMPARED )
            {
                if (!hidden(i))
                {
                    emit compare_files(qt::convert(ltree.name + "/" + list[i].items.left), qt::convert(rtree.name + "/" + list[i].items.right));
                    return;
                }
                j = i;
            }
        }
        if (j < n)
        {
            emit compare_files(qt::convert(ltree.name + "/" + list[j].items.left), qt::convert(rtree.name + "/" + list[j].items.right));
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

void DirDiffForm::on_actionSelect_Left_Only_triggered()
{
	QList< int > indices;

	for ( std::size_t i = 0, n = list.size(); i < n; ++i )
	{
		if ( !list[i].items.left.empty() && list[i].items.right.empty())
		{
			indices << i;
		}
	}

	ui->multilistview->setSelectedRows(indices);
}

void DirDiffForm::on_actionSelect_Right_Only_triggered()
{
	QList< int > indices;

	for ( std::size_t i = 0, n = list.size(); i < n; ++i )
	{
		if ( list[i].items.left.empty() && !list[i].items.right.empty())
		{
			indices << i;
		}
	}

	ui->multilistview->setSelectedRows(indices);
}

bool DirDiffForm::compare_by_items(const DirDiffForm::comparison_t &a, const DirDiffForm::comparison_t &b)
{
    const std::string l = a.items.left.empty() ? a.items.right : a.items.left;
    const std::string r = b.items.left.empty() ? b.items.right : b.items.left;
    const std::size_t nl = l.length();
    const std::size_t nr = r.length();

    // directories before files
    std::size_t il = 0; // start of path component
    std::size_t ir = 0;

    while (true)
    {
        std::size_t jl = il; // find end of path component
        std::size_t jr = ir;
        while (jl < nl && l[jl] != '/')
            ++jl;
        while (jr < nr && r[jr] != '/')
            ++jr;

        if (jl == nl && jr == nr)
        {
            // at last path component for both
            return l.compare(il, jl - il, r, ir, jr - ir) == -1;
        }
        if (jl == nl)
        {
            // first points to a file and files come after directories
            return false;
        }
        if (jr == nr)
        {
            // second points to a file and files come after directories
            return true;
        }

        // both at directories
        const int res = l.compare(il, jl - il, r, ir, jr - ir);
        if (res != 0)
            return res == -1;

        // next path component
        il = jl + 1;
        ir = jr + 1;
    }
}

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

void FileCompare::compare(
	const QString& first,
	const QString& second
)
{
    bool res;

    if ( first.endsWith(".gz") || second.endsWith(".gz"))
    {
        /// @todo gunzip to a temporary and run the file comparison
        const QByteArray data1 = gunzip(first.toStdString());
        const QByteArray data2 = gunzip(second.toStdString());

        res = (data1 == data2);
    }
    else
    {
        pbl::fs::file f(first.toStdString(), pbl::fs::file::readonly);
        pbl::fs::file g(second.toStdString(), pbl::fs::file::readonly);

        res = (f.compare(g) == 1);
    }

    emit compared(first, second, res);
}

QByteArray FileCompare::gunzip(const std::string& filename)
{
    if ( pbl::ends_with(filename, ".gz"))
    {
        QStringList l;
        l << "-c" << qt::convert(filename);

        QProcess gz;
        gz.start("gunzip", l);

        if ( !gz.waitForFinished())
        {
            return QByteArray();
        }

        return gz.readAllStandardOutput();
    }
    else
    {
        QFile file(qt::convert(filename));

        if ( !file.open(QIODevice::ReadOnly))
        {
            return QByteArray();
        }

        return file.readAll();
    }
}



int FileNameMatcher::compare(
	const std::string& a,
	const std::string& b
) const
{
	if ( a == b )
	{
        return 0;
	}

	if ( a == gzalt(b))
	{
		return 1;
	}

	if ( a == cppalt(b))
	{
		return 2;
	}

	if ( a == cgalt(b))
	{
		return 3;
	}

    return -1;
}

std::string FileNameMatcher::gzalt(const std::string& s)
{
	if ( !pbl::ends_with(s, ".gz"))
	{
		return s + ".gz";
	}

	std::string t(s, 0, s.length() - 3);

	return t;
}

std::string FileNameMatcher::cppalt(const std::string& s)
{
	if ( pbl::ends_with(s, ".cpp"))
	{
		std::string t(s, 0, s.length() - 2);

		return t;
	}

	if ( pbl::ends_with(s, ".c"))
	{
		return s + "pp";
	}

	return std::string();
}

std::string FileNameMatcher::cgalt(const std::string& s)
{
	if ( pbl::ends_with(s, ".cpp"))
	{
		std::string t(s, 0, s.length() - 2);

		return t + ".gz";
	}

	if ( pbl::ends_with(s, ".c"))
	{
		return s + "pp.gz";
	}

	if ( pbl::ends_with(s, ".c.gz"))
	{
		std::string t(s, 0, s.length() - 3);

		return t + "pp";
	}

	if ( pbl::ends_with(s, ".cpp.gz"))
	{
		std::string t(s, 0, s.length() - 5);

		return t;
	}

	return std::string();
}

void DirDiffForm::on_depthlimit_toggled(bool checked)
{
    ui->depth->setEnabled(checked);

    if (checked)
    {
        change_depth(ui->depth->value());
    }
    else
    {
        change_depth(INT_MAX);
    }
}
