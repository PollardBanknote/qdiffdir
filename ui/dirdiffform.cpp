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

#include <QTextStream>
#include <QClipboard>
#include <QMessageBox>
#include <QProcess>
#include <QFileDialog>
#include <QInputDialog>
#include <QFileSystemWatcher>
#include <QDesktopServices>
#include <QUrl>

#include "fs/fileutils.h"
#include "fs/diriter.h"
#include "fs/file.h"
#include "mysettings.h"

namespace
{
// Check that two files are equal. Files are relative to the directories passed
// to the constructor
class FileCompare : public Compare
{
public:
	FileCompare(const QString& l, const QString& r)
		: left(l), right(r)
	{

	}

	FileCompare* clone() const
	{
		return new FileCompare(*this);
	}

	/// @todo Don't unzip a file if we don't need to
	/// @todo Don't read entire files into memory when doing the file compare
	bool equal(
		const QString& lfile,
		const QString& rfile
	)
	{
		const QString first  = left + "/" + lfile;
		const QString second = right + "/" + rfile;

		if ( first.endsWith(".gz") || second.endsWith(".gz"))
		{
			const QByteArray data1 = gunzip(first);
			const QByteArray data2 = gunzip(second);

			return data1 == data2;
		}
		else
		{
			pbl::fs::file f(first.toStdString(), pbl::fs::file::readonly);
			pbl::fs::file g(second.toStdString(), pbl::fs::file::readonly);
			return f.compare(g);
		}
	}

private:
	static QByteArray gunzip(const QString& filename)
	{
		if ( filename.endsWith(".gz"))
		{
			QStringList l;
			l << "-c" << filename;

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
			QFile file(filename);

			if ( !file.open(QIODevice::ReadOnly))
			{
				return QByteArray();
			}

			return file.readAll();
		}
	}

	QString left;
	QString right;

};

class FileNameMatcher : public Matcher
{
public:
	FileNameMatcher* clone() const
	{
		return new FileNameMatcher;
	}

	int compare(
		const QString& a,
		const QString& b
	) const
	{
		if ( a == b )
		{
			return EXACT_MATCH;
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

		return DO_NOT_MATCH;
	}

private:
	// ext <=> ext.gz
	static QString gzalt(const QString& s)
	{
		if ( !s.endsWith(".gz"))
		{
			return s + ".gz";
		}

		QString t = s;
		t.chop(3);
		return t;
	}

	// c <=> cpp
	static QString cppalt(const QString& s)
	{
		if ( s.endsWith(".cpp"))
		{
			QString t = s;
			t.chop(2);
			return t;
		}

		if ( s.endsWith(".c"))
		{
			return s + "pp";
		}

		return QString();
	}

	// c <=> cpp.gz or cpp <=> c.gz
	static QString cgalt(const QString& s)
	{
		if ( s.endsWith(".cpp"))
		{
			QString t = s;
			t.chop(2);
			return t + ".gz";
		}

		if ( s.endsWith(".c"))
		{
			return s + "pp.gz";
		}

		if ( s.endsWith(".c.gz"))
		{
			QString t = s;
			t.chop(3);
			return t + "pp";
		}

		if ( s.endsWith(".cpp.gz"))
		{
			QString t = s;
			t.chop(5);
			return t;
		}

		return QString();
	}

};

}

QString lastPathComponent(const QString& s)
{
	return QString::fromStdString(pbl::fs::basename(s.toStdString()));
}

QString directoryComponent(const QString& s)
{
	return QString::fromStdString(pbl::fs::dirname(s.toStdString()));
}

DirDiffForm::DirDiffForm(QWidget* parent_) :
	QWidget(parent_),
    ui(new Ui::DirDiffForm), filter(),
    hide_left_only(false),
    hide_right_only(false), hide_identical_items(false), hide_ignored(false),
    watcher()
{
	ui->setupUi(this);

    ui->multilistview->addAction(ui->actionIgnore);
    ui->multilistview->addAction(ui->actionCopy_To_Clipboard);
    connect(ui->multilistview, SIGNAL(itemDoubleClicked(int)), SLOT(viewfiles(int)));

    connect(&derp, SIGNAL(compared(QString, QString, bool)), SLOT(items_compared(QString, QString, bool)));

    derp.setMatcher(FileNameMatcher());
    changeDirectories(derp.getLeftLocation(), derp.getRightLocation());
}

DirDiffForm::~DirDiffForm()
{
    derp.stopComparison();
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
    QStringList ss = currentText();
    const QString s1 = ss.at(0);
    const QString s2 = ss.at(1);

	if ( s1.isEmpty() && s2.isEmpty())
	{
		QMessageBox::warning(this, "No file selected", "Cannot complete action");
		return;
	}

	if ( s1.isEmpty()) // view s2
	{
        QProcess::startDetached("gvim", QStringList(s2), derp.getRightLocation());
	}
	else if ( s2.isEmpty()) // view s1
	{
        QProcess::startDetached("gvim", QStringList(s1), derp.getLeftLocation());
	}
	else // run gvimdiff
	{
		QStringList l;
        l << derp.getLeftLocation(s1)
          << derp.getRightLocation(s2);
		QProcess::startDetached(MySettings::instance().getDiffTool(), l);
	}
}

void DirDiffForm::saveAs(
	const QStringList& filenames,
	const QString&     source,
	const QString&     dest
)
{
	for ( int i = 0; i < filenames.count(); ++i )
	{
		saveAs(filenames.at(i), source, dest);
	}
}

void DirDiffForm::saveAs(
	const QString& original_filename, // a filename relative to source
	const QString& source,
	const QString& destination
)
{
	if ( !original_filename.isEmpty())
	{
		QStringList suggested_relative_path = original_filename.split(QDir::separator(), QString::SkipEmptyParts);
		suggested_relative_path.pop_back(); // remove the filename

		QDir suggested_save_directory(destination);

		bool valid_dir = true;

		for ( int i = 0; valid_dir == true && i < suggested_relative_path.size(); ++i )
		{
			valid_dir = suggested_save_directory.cd(suggested_relative_path[i]);
		}

		if ( !valid_dir )
		{
			suggested_save_directory.cd(destination);
		}

		QString suggested_filename = suggested_save_directory.absolutePath() + QDir::separator() + lastPathComponent(original_filename);
		QString save_file_name     = QFileDialog::getSaveFileName(this, "Save as", suggested_filename);

		if ( !save_file_name.isEmpty())
		{
			QString s = lastPathComponent(save_file_name);
			QString t = save_file_name;
			t.chop(s.length());
			QDir src(source);
			copyTo(src.absoluteFilePath(original_filename), t, s);
		}
	}
}

void DirDiffForm::on_copytoright_clicked()
{
    const QStringList s = ui->multilistview->getSelectedLeft();

	if ( s.isEmpty())
	{
		QMessageBox::warning(this, "No file selected", "Cannot complete action");
		return;
	}

	for ( int i = 0; i < s.count(); ++i )
	{
        copyTo(derp.getLeftLocation(s.at(i)), directoryComponent(derp.getRightLocation() + "/" + s.at(i)));
	}
}

void DirDiffForm::on_copytoleft_clicked()
{
    const QStringList s = ui->multilistview->getSelectedRight();

	if ( s.isEmpty())
	{
		QMessageBox::warning(this, "No file selected", "Cannot complete action");
		return;
	}

	for ( int i = 0; i < s.count(); ++i )
	{
        copyTo(derp.getRightLocation(s.at(i)), directoryComponent(derp.getLeftLocation() + "/" + s.at(i)));
	}
}

void DirDiffForm::on_renametoright_clicked()
{
    saveAs(ui->multilistview->getSelectedLeft(), derp.getLeftLocation(), derp.getRightLocation());
}

void DirDiffForm::on_renametoleft_clicked()
{
    saveAs(ui->multilistview->getSelectedRight(), derp.getRightLocation(), derp.getLeftLocation());
}

QString DirDiffForm::getDirectory(const QString& dir)
{
	#if 1
	return QFileDialog::getExistingDirectory(this, "Choose a directory", dir);

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
    const QString s = getDirectory(derp.getLeftLocation());

	if ( !s.isEmpty())
	{
		changeDirectories(s, QString());
	}
}

void DirDiffForm::on_openrightdir_clicked()
{
    const QString s = getDirectory(derp.getRightLocation());

	if ( !s.isEmpty())
	{
		changeDirectories(QString(), s);
	}
}

void DirDiffForm::viewfiles(
       int x_
)
{
    if ( x_ >= 0 )
    {
        const QString s1 = list[x_].items.left;
        const QString s2 = list[x_].items.right;

        if ( s1.isEmpty())
        {
            QProcess::startDetached("gvim",
                QStringList(derp.getRightLocation(s2)));
        }
        else if ( s2.isEmpty())
        {
            QProcess::startDetached("gvim",
                QStringList(derp.getLeftLocation(s1)));
        }
        else // run gvimdiff
        {
            QStringList l;
            l << derp.getLeftLocation(s1)
              << derp.getRightLocation(s2);
            QProcess::startDetached(MySettings::instance().getDiffTool(), l);
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

// directories have not changed, but list has
void DirDiffForm::on_depth_valueChanged(int d)
{
    QStringList       reml   = derp.getLeftRelativeFileNames();
    QStringList       remr   = derp.getRightRelativeFileNames();
    QPair< QStringList, QStringList > bothfiles = derp.setDepth(d);
    {
        const QStringList files = bothfiles.first;

		for ( int i = 0; i < files.count(); ++i )
		{
            reml.removeAll(files.at(i));
		}

        updateLeft(files, reml);
	}
	{
        const QStringList files = bothfiles.second;

		for ( int i = 0; i < files.count(); ++i )
		{
            remr.removeAll(files.at(i));
		}

        updateRight(files, remr);
	}
}

void DirDiffForm::changeDirectories(
	const QString& left,
	const QString& right
)
{
	// stop the watcher, and clear the comparisons we know
	stopDirectoryWatcher();

	// change to the new directories
	if ( !left.isEmpty())
	{
        derp.setLeftLocation(left);
        ui->openleftdir->setText(derp.getLeftName());
	}

	if ( !right.isEmpty())
	{
        derp.setRightLocation(right);
        ui->openrightdir->setText(derp.getRightName());
	}

	const int         depth_     = ui->depth->value();
    const QPair< QStringList, QStringList > bothfiles = derp.setDepth(depth_);
    const QStringList leftfiles  = bothfiles.first;
    const QStringList rightfiles = bothfiles.second;

	{
		/// @todo If left and right are the same, don't do both
		/// @todo If a dir hasn't changed, don't go through it
		// refresh the compareview

		when = QDateTime::currentDateTime();

        setLeftAndRight(derp.getLeftLocation(), derp.getRightLocation(), leftfiles, rightfiles);
        setComparison(FileCompare(derp.getLeftLocation(), derp.getRightLocation()));
	}

	startDirectoryWatcher();

	if( !ui->autoRefresh->isChecked())
	{
		stopDirectoryWatcher();
	}
}

void DirDiffForm::copyTo(
	const QString& file,
	const QString& destdir
)
{
	copyTo(file, destdir, lastPathComponent(file));
}

void DirDiffForm::copyTo(
	const QString& file,
	const QString& destdir,
	const QString& newname
)
{
	const QString s = destdir + "/" + newname;

	if ( QFile::exists(s))
	{
		if ( QMessageBox::question(this, newname + " already exists", "Do you want to overwrite the destination?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::No )
		{
			return;
		}
	}

	if ( pbl::fs::copy_file(file.toStdString(), s.toStdString()))
	{
		fileChanged(s);
	}
}

void DirDiffForm::stopDirectoryWatcher()
{
	if( watcher != NULL)
	{
		delete watcher;
		watcher = NULL;
	}
}

void DirDiffForm::startDirectoryWatcher()
{
	stopDirectoryWatcher();

	// create new file system watcher
	QStringList dirlist;
	dirlist << ldir.getDirectories()
			<< rdir.getDirectories();

	// Note: there is a bug that causes a crash if QFileSystemWatcher gets the
	// same path twice
	dirlist.removeDuplicates();

	watcher = new QFileSystemWatcher(dirlist, this);
	connect(watcher, SIGNAL(directoryChanged(QString)), SLOT(contentsChanged(QString)));
}

// File system has notified us of a change in one of our directories
/// @bug Watch any new subdirectories that popped up
/// @todo added_or_changed should not include files that haven't changed
void DirDiffForm::contentsChanged(QString dirname_)
{
	QDir          eventdir(dirname_);
	const QString dirname = dirName(eventdir); // absolute path of dir

	// absolute path of every file below dirname
    if ( dirname.startsWith(derp.getLeftLocation()))
	{
        DirectoryContents::update_t u = derp.updateLeft(dirname);

		for ( int i = 0, n = u.changed.count(); i < n; ++i )
		{
            QFileInfo fi(derp.getLeftLocation(u.changed.at(i)));

			if ( fi.lastModified() > when )
			{
				u.added << u.changed.at(i);
			}
		}

        updateLeft(u.added, u.removed);
	}

    if ( dirname.startsWith(derp.getRightLocation()))
	{
        DirectoryContents::update_t u = derp.updateRight(dirname);

		for ( int i = 0, n = u.changed.count(); i < n; ++i )
		{
            QFileInfo fi(derp.getRightLocation(u.changed.at(i)));

			if ( fi.lastModified() > when )
			{
				u.added << u.changed.at(i);
			}
		}

        updateRight(u.added, u.removed);
	}
}

// A single file has been changed/or added. Everything else stayed the same
// file is an absolute path
void DirDiffForm::fileChanged(QString file)
{
    if ( file.startsWith(derp.getLeftLocation()))
	{
        updateLeft(QStringList(derp.getLeftRelativeFilePath(file)));
	}

    if ( file.startsWith(derp.getRightLocation()))
	{
        updateRight(QStringList(derp.getRightRelativeFilePath(file)));
	}
}

// sometimes QDir::dirName returns ".", so we use this instead
// Includes the trailing backslash. Simplified.
QString DirDiffForm::dirName(const QDir& dir)
{
	QString s = QDir::cleanPath(dir.absolutePath());

	if ( !s.endsWith(QDir::separator()))
	{
		s += QDir::separator();
	}

	return s;
}

void DirDiffForm::on_refresh_clicked()
{
    changeDirectories(derp.getLeftLocation(), derp.getRightLocation());
}

void DirDiffForm::on_swap_clicked()
{
	/// @todo does a lot of work unnecessarily
    changeDirectories(derp.getRightLocation(), derp.getLeftLocation());
}

void DirDiffForm::on_openright_clicked()
{
    QDesktopServices::openUrl(QUrl("file://" + derp.getRightLocation()));
}

void DirDiffForm::on_openleft_clicked()
{
    QDesktopServices::openUrl(QUrl("file://" + derp.getRightLocation()));
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
    switch (index)
    {
    case 0:
        clearFilter();
        break;
    case 1:
        setFilter(QRegExp(".*(cpp|h)"));
        break;
    }
}

void DirDiffForm::on_filter_editTextChanged(const QString &arg1)
{
    QRegExp rx(arg1);

    setFilter(rx);
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
    if ( hide_identical_items && list[i].compared && list[i].same )
    {
        hideitem = true;
    }

    // Hide items that don't match the current filter
    if ( !filter.isEmpty() && !filter.exactMatch(list[i].items.left) && !filter.exactMatch(list[i].items.right))
    {
        hideitem = true;
    }

    return hideitem;
}

void DirDiffForm::applyFilters()
{
    // save the currently selected item
    const int   sel              = ui->multilistview->currentRow();
    std::size_t last_low_shown   = NOT_FOUND;
    std::size_t first_high_shown = NOT_FOUND;

    // for each item, check if it is shown or not, and adjust font
    for ( std::size_t i = 0, n = list.size(); i < n; ++i )
    {
        const bool hideitem = hidden(i);

        ui->multilistview->style(i, hideitem, list[i].ignore, list[i].unmatched(), list[i].compared, list[i].same);

        // find the shown items before and after the selected
        if ( sel >= 0 && !hideitem )
        {
            if ( i <= static_cast< unsigned >( sel ))
            {
                last_low_shown = i;
            }

            if ( i >= static_cast< unsigned >( sel ) && first_high_shown == NOT_FOUND )
            {
                first_high_shown = i;
            }
        }
    }

    // select the correct item
    if ( first_high_shown != NOT_FOUND )
    {
        ui->multilistview->changesel(first_high_shown);
    }
    else if ( last_low_shown != NOT_FOUND )
    {
        ui->multilistview->changesel(last_low_shown);
    }
    else
    {
        ui->multilistview->clearSelection();
    }
}

void DirDiffForm::items_compared(
    QString first,
    QString second,
    bool    equal
)
{
    for ( std::size_t i = 0, n = list.size(); i < n; ++i )
    {
        if ( list[i].items.left == first && list[i].items.right == second )
        {
            list[i].compared = true;
            list[i].same     = equal;
            applyFilters();
            return;
        }
    }
}

void DirDiffForm::clearFilter()
{
    setFilter(QRegExp());
}

void DirDiffForm::setFilter(const QRegExp & r)
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
    int idx = ui->multilistview->currentRow();

    if ( idx >= 0 )
    {
        list[idx].ignore = true;
        applyFilters();
    }
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
            ts << list[i].items.left << '\t' << list[i].items.right << '\t';

            // result of comparison
            if ( list[i].left_only())
            {
                ts << "Left Only";
            }
            else if ( list[i].right_only())
            {
                ts << "Right Only";
            }
            else if ( list[i].compared )
            {
                if ( list[i].same )
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

void DirDiffForm::setLeftAndRight(
    const QString&     leftname_,
    const QString&     rightname_,
    const QStringList& leftitems,
    const QStringList& rightitems
)
{
    derp.stopComparison();

    {
        // Update internal list
        QStringList ignore_left;
        QStringList ignore_right;

        for ( std::size_t i = 0, n = list.size(); i < n; ++i )
        {
            if ( list[i].ignore )
            {
                if ( !list[i].items.left.isEmpty())
                {
                    ignore_left << list[i].items.left;
                }

                if ( !list[i].items.right.isEmpty())
                {
                    ignore_right << list[i].items.right;
                }
            }
        }

        std::vector< items_t > filepairs = match(leftitems, rightitems);

        std::vector< comparison_t > temp;

        for ( std::size_t i = 0, n = filepairs.size(); i < n; ++i )
        {
            comparison_t c = { filepairs[i], false, false, false };

            if ( ignore_left.contains(filepairs[i].left) || ignore_right.contains(filepairs[i].right))
            {
                c.ignore = true;
            }

            temp.push_back(c);
        }

        list      = temp;
        leftname  = leftname_;
        rightname = rightname_;
    }

    QList< QPair< QString, QString > > items;

    for ( std::size_t i = 0, n = list.size(); i < n; ++i )
    {
        items.append(qMakePair(list[i].items.left, list[i].items.right));
    }

    ui->multilistview->setItems(items);

    applyFilters();
}

std::vector< items_t > DirDiffForm::match(
    const QStringList& leftitems,
    const QStringList& rightitems
) const
{
    std::map< int, std::vector< items_t > > score;

    for ( int il = 0, nl = leftitems.count(); il < nl; ++il )
    {
        for ( int ir = 0, nr = rightitems.count(); ir < nr; ++ir )
        {
            score[derp.match(leftitems.at(il), rightitems.at(ir))].push_back(
                items_t(leftitems.at(il), rightitems.at(ir)));
        }
    }

    std::vector< items_t > matching;

    QStringList leftmatched;
    QStringList rightmatched;

    for ( std::map< int, std::vector< items_t > >::iterator it = score.begin(); it != score.end(); ++it )
    {
        if ( it->first != Matcher::DO_NOT_MATCH )
        {
            std::vector< items_t > l = it->second;

            for ( std::size_t i = 0; i < l.size(); ++i )
            {
                if ( !leftmatched.contains(l[i].left) && !rightmatched.contains(l[i].right))
                {
                    matching.push_back(l[i]);
                    leftmatched << l[i].left;
                    rightmatched << l[i].right;
                }
            }
        }
    }

    for ( int i = 0, n = leftitems.count(); i < n; ++i )
    {
        if ( !leftmatched.contains(leftitems.at(i)))
        {
            matching.push_back(items_t(leftitems.at(i), QString()));
        }
    }

    for ( int i = 0, n = rightitems.count(); i < n; ++i )
    {
        if ( !rightmatched.contains(rightitems.at(i)))
        {
            matching.push_back(items_t(QString(), rightitems.at(i)));
        }
    }

    std::sort(matching.begin(), matching.end());

    return matching;
}

void DirDiffForm::setComparison(const Compare& comp)
{
    if ( Compare * p = comp.clone())
    {
        derp.stopComparison();
        derp.setCompare(p);

        for ( std::size_t i = 0; i < list.size(); ++i )
        {
            list[i].compared = false;
        }

        startComparison();

    }
}

void DirDiffForm::startComparison()
{
    std::vector< items_t > matches;

    for ( std::size_t i = 0, n = list.size(); i < n; ++i )
    {
        if ( !list[i].items.left.isEmpty() && !list[i].items.right.isEmpty() && !list[i].compared )
        {
            matches.push_back(list[i].items);
        }
    }

    if ( !matches.empty())
    {
        derp.stopComparison();
        derp.startWorker(matches);
    }
}

QStringList DirDiffForm::getAllLeft() const
{
    QStringList l;

    for ( std::size_t i = 0, n = list.size(); i < n; ++i )
    {
        if ( !list[i].items.left.isEmpty())
        {
            l << list[i].items.left;
        }
    }

    return l;
}

QStringList DirDiffForm::getAllRight() const
{
    QStringList l;

    for ( std::size_t i = 0, n = list.size(); i < n; ++i )
    {
        if ( !list[i].items.right.isEmpty())
        {
            l << list[i].items.right;
        }
    }

    return l;
}

void DirDiffForm::updateLeft(
    const QStringList& added_or_changed,
    const QStringList& remove
)
{
    QStringList oldleft;

    for ( std::size_t i = 0, n = list.size(); i < n; ++i )
    {
        if ( !list[i].items.left.isEmpty())
        {
            if ( remove.contains(list[i].items.left))
            {
                list[i].items.left = QString();
                list[i].compared   = false;
                list[i].same       = false;
                ui->multilistview->clearLeft(i);
            }
            else if ( added_or_changed.contains(list[i].items.left))
            {
                list[i].compared = false;
                list[i].same     = false;
            }

            oldleft << list[i].items.left;
        }
    }

    for ( int i = 0, n = added_or_changed.count(); i < n; ++i )
    {
        const QString item = added_or_changed.at(i);

        // was item added?
        if ( !oldleft.contains(item))
        {
            items_t x(item, QString());

            std::size_t j = 0;

            while ( j < list.size() && list[j].items < x )
            {
                ++j;
            }

            comparison_t y = { x, false, false, false };

            // insert into the list
            list.insert(list.begin() + j, y);

            ui->multilistview->insert(j, item, QString());
        }
    }

    // remove empty items
    for ( std::size_t i = 0; i < list.size();)
    {
        if ( list[i].items.right.isEmpty() && list[i].items.left.isEmpty())
        {
            ui->multilistview->remove(i);
            list.erase(list.begin() + i);
        }
        else
        {
            ++i;
        }
    }

    rematch();
}

void DirDiffForm::updateRight(
    const QStringList& added_or_changed,
    const QStringList& remove
)
{
    QStringList oldright;

    for ( std::size_t i = 0, n = list.size(); i < n; ++i )
    {
        if ( !list[i].items.right.isEmpty())
        {
            if ( remove.contains(list[i].items.right))
            {
                list[i].items.right = QString();
                list[i].compared    = false;
                list[i].same        = false;
                ui->multilistview->clearRight(i);
            }
            else if ( added_or_changed.contains(list[i].items.right))
            {
                list[i].compared = false;
                list[i].same     = false;
            }

            oldright << list[i].items.right;
        }
    }

    for ( int i = 0, n = added_or_changed.count(); i < n; ++i )
    {
        const QString item = added_or_changed.at(i);

        // was item added?
        if ( !oldright.contains(item))
        {
            items_t x(QString(), item);

            std::size_t j = 0;

            while ( j < list.size() && list[j].items < x )
            {
                ++j;
            }

            comparison_t y = { x, false, false, false };

            // insert into the list
            list.insert(list.begin() + j, y);

            ui->multilistview->insert(j, QString(), item);
        }
    }

    // remove empty items
    for ( std::size_t i = 0; i < list.size();)
    {
        if ( list[i].items.left.isEmpty() && list[i].items.right.isEmpty())
        {
            ui->multilistview->remove(i);
            list.erase(list.begin() + i);
        }
        else
        {
            ++i;
        }
    }

    rematch();
}

void DirDiffForm::rematch()
{
    // rematch filenames
    std::vector< items_t > matching = match(getAllLeft(), getAllRight());

    // erase all existing rows that are not in the new matching
    for ( std::size_t i = 0; i < list.size();)
    {
        std::size_t j = 0;

        while ( j < matching.size() && list[i].items != matching[j] )
        {
            ++j;
        }

        if ( j == matching.size())
        {
            ui->multilistview->remove(i);
            list.erase(list.begin() + i);
        }
        else
        {
            ++i;
        }
    }

    // add all items in matching that aren't in the current view
    for ( std::size_t i = 0; i < matching.size(); ++i )
    {
        if ( i >= list.size() || matching[i] != list[i].items )
        {
            comparison_t c = { matching[i], false, false, false };
            list.insert(list.begin() + i, c);

            ui->multilistview->insert(i, matching[i].left, matching[i].right);
        }
    }

    startComparison();
    applyFilters();

}

QStringList DirDiffForm::currentText() const
{
    return ui->multilistview->currentText();
}

void DirDiffForm::on_autoRefresh_stateChanged(int state)
{
	if( state == Qt::Checked)
	{
		startDirectoryWatcher();
	}
	else
	{
		stopDirectoryWatcher();
	}

}
