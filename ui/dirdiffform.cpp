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

#include "util/file.h"
#include "util/strings.h"
#include "util/containers.h"

#include "compare.h"
#include "matcher.h"
#include "mysettings.h"
#include "qutilities/icons.h"
#include "qutilities/convert.h"

namespace
{

// Check that two files are equal. Files are relative to the directories passed
// to the constructor
class FileCompare : public Compare
{
public:
	FileCompare(
        const std::string& l,
        const std::string& r
	)
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
        const std::string& lfile,
        const std::string& rfile
	)
	{
        const std::string first  = left + "/" + lfile;
        const std::string second = right + "/" + rfile;

        if ( pbl::ends_with(first, ".gz") || pbl::ends_with(second, ".gz"))
		{
			const QByteArray data1 = gunzip(first);
			const QByteArray data2 = gunzip(second);

			return data1 == data2;
		}
		else
		{
            pbl::fs::file f(first, pbl::fs::file::readonly);
            pbl::fs::file g(second, pbl::fs::file::readonly);

            return f.compare(g);
		}
	}

private:
    static QByteArray gunzip(const std::string& filename)
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

    std::string left;
    std::string right;

};

class FileNameMatcher : public Matcher
{
public:
	FileNameMatcher* clone() const
	{
		return new FileNameMatcher;
	}

	int compare(
        const std::string& a,
        const std::string& b
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
    static std::string gzalt(const std::string& s)
	{
        if ( !pbl::ends_with(s, ".gz"))
		{
			return s + ".gz";
		}

        std::string t(s, 0, s.length() - 3);
		return t;
	}

	// c <=> cpp
    static std::string cppalt(const std::string& s)
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

	// c <=> cpp.gz or cpp <=> c.gz
    static std::string cgalt(const std::string& s)
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

};

}

QString lastPathComponent(const QString& s)
{
    return qt::convert(cpp::filesystem::basename(qt::convert(s)));
}

QString directoryComponent(const QString& s)
{
    return qt::convert(cpp::filesystem::dirname(qt::convert(s)));
}

DirDiffForm::DirDiffForm(QWidget* parent_) :
	QWidget(parent_),
	ui(new Ui::DirDiffForm), filter(),
	hide_left_only(false),
	hide_right_only(false), hide_identical_items(false), hide_ignored(false),
	watcher()
{
	ui->setupUi(this);
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
    const int r = ui->multilistview->currentRow();
    if (r >= 0)
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
            QProcess::startDetached(settings.getEditor(), QStringList(qt::convert(s2)), qt::convert(derp.getRightLocation()));
        }
        else if ( s2.empty()) // view s1
        {
            QProcess::startDetached(settings.getEditor(), QStringList(qt::convert(s1)), qt::convert(derp.getLeftLocation()));
        }
        else
        {
            QStringList l;
            l << QString::fromStdString(derp.getLeftLocation(s1))
              << QString::fromStdString(derp.getRightLocation(s2));
            QProcess::startDetached(settings.getDiffTool(), l);
        }
    }
}

void DirDiffForm::saveAs(
    const std::vector< std::string >& filenames,
    const std::string&     source,
    const std::string&     dest
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
            QDir src(qt::convert(source));
            std::pair< bool, overwrite_t > res = copyTo(qt::convert(src.absoluteFilePath(qt::convert(original_filename))), qt::convert(t), qt::convert(s), OVERWRITE_ASK);
            if (res.first)
                fileChanged(qt::convert(save_file_name));
		}
	}
}

void DirDiffForm::on_copytoright_clicked()
{
    const QList<int> indices = ui->multilistview->selectedRows();

    bool nonempty = false;

    overwrite_t overwrite = OVERWRITE_ASK;

    std::vector< std::string > changed;

    for ( int i = 0, n = indices.count(); i < n; ++i )
	{
        const std::string rel = list[indices[i]].items.left;
        if (!rel.empty())
        {
            const std::string source_file =  derp.getLeftLocation(rel);
            const std::string dest_file = derp.getRightLocation() + "/" + rel;
            const std::string dest_dir = cpp::filesystem::dirname(dest_file);
            const std::string file_name = cpp::filesystem::basename(source_file);
            std::pair< bool, overwrite_t > res = copyTo(source_file, dest_dir, file_name, overwrite);
            if (res.first)
                changed.push_back(dest_file);
            overwrite = res.second;
            nonempty = true;
        }
	}

    if (!nonempty)
    {
        QMessageBox::warning(this, "No file selected", "Cannot complete action");
        return;
    }
    if (!changed.empty())
        filesChanged(changed);
}

void DirDiffForm::on_copytoleft_clicked()
{
    const QList<int> indices = ui->multilistview->selectedRows();

    bool nonempty = false;

    overwrite_t overwrite = OVERWRITE_ASK;

    std::vector< std::string > changed;

    for ( int i = 0, n = indices.count(); i < n; ++i )
    {
        const std::string rel = list[indices[i]].items.right;
        if (!rel.empty())
        {
            std::string source_file = derp.getRightLocation(rel);
            std::string dest_file = derp.getLeftLocation() + "/" + rel;
            std::string dest_dir = cpp::filesystem::dirname(dest_file);
            std::string file_name = cpp::filesystem::basename(source_file);
            std::pair< bool, overwrite_t > res = copyTo(source_file, dest_dir, file_name, overwrite);
            if (res.first)
                changed.push_back(dest_file);
            overwrite = res.second;
            nonempty = true;
        }
    }

    if (!nonempty)
    {
        QMessageBox::warning(this, "No file selected", "Cannot complete action");
        return;
    }
    if (!changed.empty())
        filesChanged(changed);
}

void DirDiffForm::on_renametoright_clicked()
{
    const QList<int> indices = ui->multilistview->selectedRows();
    std::vector< std::string > files;
    for (int i = 0, n = indices.count(); i < n; ++i)
        if (!list[indices.at(i)].items.left.empty())
            files.push_back(list[indices.at(i)].items.left);
    saveAs(files, derp.getLeftLocation(), derp.getRightLocation());
}

void DirDiffForm::on_renametoleft_clicked()
{
    const QList<int> indices = ui->multilistview->selectedRows();
    std::vector< std::string > files;
    for (int i = 0, n = indices.count(); i < n; ++i)
        if (!list[indices.at(i)].items.right.empty())
            files.push_back(list[indices.at(i)].items.right);
    saveAs(files, derp.getRightLocation(), derp.getLeftLocation());
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
    const QString s = getDirectory(qt::convert(derp.getLeftLocation()));

	if ( !s.isEmpty())
	{
        changeDirectories(qt::convert(s), std::string());
	}
}

void DirDiffForm::on_openrightdir_clicked()
{
    const QString s = getDirectory(qt::convert(derp.getRightLocation()));

	if ( !s.isEmpty())
	{
        changeDirectories(std::string(), qt::convert(s));
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
            QProcess::startDetached(settings.getEditor(),
                QStringList(qt::convert(derp.getRightLocation(s2))));
		}
        else if ( s2.empty())
		{
            QProcess::startDetached(settings.getEditor(),
                QStringList(qt::convert(derp.getLeftLocation(s1))));
		}
        else
		{
			QStringList l;
            l << qt::convert(derp.getLeftLocation(s1))
              << qt::convert(derp.getRightLocation(s2));
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

// directories have not changed, but list has
void DirDiffForm::on_depth_valueChanged(int d)
{
    std::set< std::string > reml = pbl::make_set(derp.getLeftRelativeFileNames());
    std::set< std::string > remr = pbl::make_set(derp.getRightRelativeFileNames());

    std::pair< std::vector< std::string >, std::vector< std::string > > bothfiles = derp.setDepth(d);
	{

        const std::vector< std::string > files = bothfiles.first;

        for ( std::size_t i = 0; i < files.size(); ++i )
		{
            reml.erase(files[i]);
		}

        updateLeft(files, std::vector< std::string >(reml.begin(), reml.end()));
	}
	{
        const std::vector< std::string > files = bothfiles.second;

        for ( std::size_t i = 0; i < files.size(); ++i )
		{
            remr.erase(files[i]);
		}

        updateRight(files, std::vector< std::string >(remr.begin(), remr.end()));
	}
}

void DirDiffForm::changeDirectories(
    const std::string& left,
    const std::string& right
)
{
	// stop the watcher, and clear the comparisons we know
	stopDirectoryWatcher();

	// change to the new directories
    if ( !left.empty())
	{
		derp.setLeftLocation(left);
        ui->openleftdir->setText(qt::convert(derp.getLeftName()));
	}

    if ( !right.empty())
	{
		derp.setRightLocation(right);
        ui->openrightdir->setText(qt::convert(derp.getRightName()));
	}

	const int                               depth_     = ui->depth->value();
    const std::pair< std::vector< std::string >, std::vector< std::string > > bothfiles  = derp.setDepth(depth_);
    const std::vector< std::string >                       leftfiles  = bothfiles.first;
    const std::vector< std::string >                       rightfiles = bothfiles.second;

	{
		/// @todo If left and right are the same, don't do both
		/// @todo If a dir hasn't changed, don't go through it
		// refresh the compareview

		when = QDateTime::currentDateTime();

		setLeftAndRight(derp.getLeftLocation(), derp.getRightLocation(), leftfiles, rightfiles);
	}

	startDirectoryWatcher();

	if ( !ui->autoRefresh->isChecked())
	{
		stopDirectoryWatcher();
	}
}

std::pair< bool, DirDiffForm::overwrite_t> DirDiffForm::copyTo(
    const std::string& file,
    const std::string& destdir,
    const std::string& newname,
        overwrite_t overwrite
)
{
    const std::string s = destdir + "/" + newname;

    if ( cpp::filesystem::exists(s))
	{
        switch (overwrite)
        {
        case OVERWRITE_ASK:
        {
            QMessageBox::StandardButton res = QMessageBox::question(this, qt::convert(newname) + " already exists", "Do you want to overwrite the destination?", QMessageBox::YesToAll | QMessageBox::NoToAll | QMessageBox::Yes | QMessageBox::No);

            if (res == QMessageBox::No )
            {
                return std::make_pair(false, OVERWRITE_ASK);
            }
            else if (res == QMessageBox::NoToAll)
            {
                return std::make_pair(false, OVERWRITE_NO);
            }
            else if (res == QMessageBox::YesToAll)
                overwrite = OVERWRITE_YES;
        }
            break;
        case OVERWRITE_YES:
            break;
        case OVERWRITE_NO:
            return std::make_pair(false, OVERWRITE_NO);
        }
	}

    if ( cpp::filesystem::copy_file(file, s, copy_options::overwrite_existing))
	{
        return std::make_pair(true, overwrite);
	}
    else
        return std::make_pair(false, overwrite);
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
	stopDirectoryWatcher();

	// create new file system watcher
    std::vector< std::string > dirlist = derp.getDirectories();

    watcher = new QFileSystemWatcher(qt::convert(dirlist), this);
	connect(watcher, SIGNAL(directoryChanged(QString)), SLOT(contentsChanged(QString)));
}

// File system has notified us of a change in one of our directories
/// @bug Watch any new subdirectories that popped up
/// @todo added_or_changed should not include files that haven't changed
void DirDiffForm::contentsChanged(QString dirname_)
{
	QDir          eventdir(dirname_);
    const std::string dirname = qt::convert(dirName(eventdir)); // absolute path of dir

	// absolute path of every file below dirname
    if ( pbl::starts_with(dirname,derp.getLeftLocation()))
	{
		DirectoryContents::update_t u = derp.updateLeft(dirname);

        for ( std::size_t i = 0, n = u.changed.size(); i < n; ++i )
		{
            QFileInfo fi(qt::convert(derp.getLeftLocation(u.changed[i])));

			if ( fi.lastModified() > when )
			{
                u.added.push_back(u.changed[i]);
			}
		}

		updateLeft(u.added, u.removed);
	}

    if ( pbl::starts_with(dirname,derp.getRightLocation()))
	{
		DirectoryContents::update_t u = derp.updateRight(dirname);

        for ( std::size_t i = 0, n = u.changed.size(); i < n; ++i )
		{
            QFileInfo fi(qt::convert(derp.getRightLocation(u.changed[i])));

			if ( fi.lastModified() > when )
			{
                u.added.push_back(u.changed[i]);
			}
		}

		updateRight(u.added, u.removed);
	}
}

// A single file has been changed/or added. Everything else stayed the same
// file is an absolute path
void DirDiffForm::fileChanged(const std::string& file)
{
    if ( pbl::starts_with(file, derp.getLeftLocation()))
	{
        updateLeft(derp.getLeftRelativeFilePath(file));
	}

    if ( pbl::starts_with(file, derp.getRightLocation()))
	{
        updateRight(derp.getRightRelativeFilePath(file));
	}
}

void DirDiffForm::filesChanged(const std::vector< std::string > & files)
{
    std::vector< std::string > l;
    std::vector< std::string > r;

    std::string lloc = derp.getLeftLocation();
    std::string rloc = derp.getRightLocation();

    for (std::size_t i = 0, n = files.size(); i < n; ++i)
    {
        std::string file = files[i];

        if ( pbl::starts_with(file, lloc))
        {
            l.push_back(derp.getLeftRelativeFilePath(file));
        }

        if ( pbl::starts_with(file, rloc))
        {
            r.push_back(derp.getRightRelativeFilePath(file));
        }
    }

    updateLeft(l);
    updateRight(r);
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
    QDesktopServices::openUrl(QUrl("file://" + qt::convert(derp.getRightLocation())));
}

void DirDiffForm::on_openleft_clicked()
{
    QDesktopServices::openUrl(QUrl("file://" + qt::convert(derp.getRightLocation())));
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
	if ( hide_identical_items && list[i].compared && list[i].same )
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
	
	bool seen_selected = false;
	const std::size_t n = list.size();
	std::size_t first_unselected_after_selected = n;
	std::size_t last_unselected = n;
	
	// for each item, check if it is shown or not, and adjust font
	for ( std::size_t i = 0; i < n; ++i )
	{
		const bool hideitem = hidden(i);

        ui->multilistview->style(i, list[i].ignore, list[i].unmatched(), list[i].compared, list[i].same);
        ui->multilistview->setRowHidden(i, hideitem);
		
		if (sel.contains(i))
		{
			seen_selected = true;
			if (!hideitem)
				new_selection.append(i);
		}
		else
		{
			if (!hideitem)
			{
				if (seen_selected && first_unselected_after_selected == n)
					first_unselected_after_selected = i;
				last_unselected = i;
			}
		}
	}

	if (new_selection.isEmpty() && !sel.isEmpty())
	{
		// select the first visible row after the selection begins
		if (first_unselected_after_selected != n)
			new_selection << first_unselected_after_selected;
		else if (last_unselected != n)
			new_selection << last_unselected;
	}
	
	ui->multilistview->setSelectedRows(new_selection);
}

void DirDiffForm::items_compared(
    std::string first,
    std::string second,
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
	
	bool some_ignored = false;
	bool some_not_ignored = false;

	for (int i = 0, n = l.count(); i < n; ++i)
	{
		const int idx = l.at(i);
		
		if (idx >= 0 && static_cast<unsigned>(idx) < list.size())
		{
			if (list[idx].ignore)
				some_ignored = true;
			else
				some_not_ignored = true;
		}
	}
	
	bool all_ignored = some_ignored && !some_not_ignored;
	
	bool ignore = (all_ignored ? false : true);
	
	for (int i = 0, n = l.count(); i < n; ++i)
	{
		const int idx = l.at(i);
		if (idx >= 0 && static_cast<unsigned>(idx) < list.size())
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
    const std::string&     leftname_,
    const std::string&     rightname_,
    const std::vector< std::string >& leftitems,
    const std::vector< std::string >& rightitems
)
{
	derp.stopComparison();

	{
		// Update internal list
        std::set< std::string> ignore_left;
        std::set< std::string > ignore_right;

		for ( std::size_t i = 0, n = list.size(); i < n; ++i )
		{
			if ( list[i].ignore )
			{
                if ( !list[i].items.left.empty())
				{
                    ignore_left.insert(list[i].items.left);
				}

                if ( !list[i].items.right.empty())
				{
                    ignore_right.insert(list[i].items.right);
				}
			}
		}

		std::vector< items_t > filepairs = match(leftitems, rightitems);

		std::vector< comparison_t > temp;

		for ( std::size_t i = 0, n = filepairs.size(); i < n; ++i )
		{
			comparison_t c = { filepairs[i], false, false, false };

            if ( ignore_left.count(filepairs[i].left) != 0 || ignore_right.count(filepairs[i].right)!= 0)
			{
				c.ignore = true;
			}

			temp.push_back(c);
		}

		list      = temp;
        leftname  = qt::convert(leftname_);
        rightname = qt::convert(rightname_);
	}

    ui->multilistview->clear();
	for ( std::size_t i = 0, n = list.size(); i < n; ++i )
	{
        QStringList items;
        items << qt::convert(list[i].items.left) << qt::convert(list[i].items.right);
        ui->multilistview->addItem(items);
	}

	applyFilters();
	setComparison(FileCompare(leftname_, rightname_));
}

std::vector< items_t > DirDiffForm::match(
    const std::vector< std::string >& leftitems,
    const std::vector< std::string >& rightitems
) const
{
	std::map< int, std::vector< items_t > > score;

    for ( std::size_t il = 0, nl = leftitems.size(); il < nl; ++il )
	{
        for ( std::size_t ir = 0, nr = rightitems.size(); ir < nr; ++ir )
		{
            score[derp.match(leftitems[il], rightitems[ir])].push_back(
                items_t(leftitems[il], rightitems[ir]));
		}
	}

	std::vector< items_t > matching;

    std::set< std::string > leftmatched;
    std::set< std::string > rightmatched;

	for ( std::map< int, std::vector< items_t > >::iterator it = score.begin(); it != score.end(); ++it )
	{
		if ( it->first != Matcher::DO_NOT_MATCH )
		{
			std::vector< items_t > l = it->second;

			for ( std::size_t i = 0; i < l.size(); ++i )
			{
                if ( leftmatched.count(l[i].left) == 0 && rightmatched.count(l[i].right) == 0)
				{
					matching.push_back(l[i]);
                    leftmatched.insert(l[i].left);
                    rightmatched.insert(l[i].right);
				}
			}
		}
	}

    for ( std::size_t i = 0, n = leftitems.size(); i < n; ++i )
	{
        if ( leftmatched.count(leftitems[i]) == 0)
		{
            matching.push_back(items_t(leftitems[i], std::string()));
		}
	}

    for ( std::size_t i = 0, n = rightitems.size(); i < n; ++i )
	{
        if ( rightmatched.count(rightitems[i]) == 0)
		{
            matching.push_back(items_t(std::string(), rightitems[i]));
		}
	}

	std::sort(matching.begin(), matching.end());

	return matching;
}

void DirDiffForm::setComparison(const Compare& comp)
{
	if ( Compare* p = comp.clone())
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
        if ( !list[i].items.left.empty() && !list[i].items.right.empty() && !list[i].compared )
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

std::vector< std::string> DirDiffForm::getAllLeft() const
{
    std::vector< std::string> l;

	for ( std::size_t i = 0, n = list.size(); i < n; ++i )
	{
        if ( !list[i].items.left.empty())
		{
            l.push_back(list[i].items.left);
		}
	}

	return l;
}

std::vector< std::string> DirDiffForm::getAllRight() const
{
    std::vector< std::string> l;

	for ( std::size_t i = 0, n = list.size(); i < n; ++i )
	{
        if ( !list[i].items.right.empty())
		{
            l.push_back(list[i].items.right);
		}
	}

	return l;
}

void DirDiffForm::updateLeft(const std::string &added_or_changed)
{
    std::vector< std::string > v(1, added_or_changed);
    updateLeft(v);
}

void DirDiffForm::updateLeft(
    const std::vector< std::string >& added_or_changed,
    const std::vector< std::string >& remove
)
{
    std::set< std::string > oldleft;

	for ( std::size_t i = 0, n = list.size(); i < n; ++i )
	{
        if ( !list[i].items.left.empty())
		{
            if ( pbl::contains(remove,list[i].items.left))
			{
                list[i].items.left = std::string();
				list[i].compared   = false;
				list[i].same       = false;
                ui->multilistview->clearText(0, i);
			}
            else if ( pbl::contains(added_or_changed,list[i].items.left))
			{
				list[i].compared = false;
				list[i].same     = false;
			}

            oldleft.insert(list[i].items.left);
		}
	}

    for ( std::size_t i = 0, n = added_or_changed.size(); i < n; ++i )
	{
        const std::string item = added_or_changed.at(i);

		// was item added?
        if ( oldleft.count(item) == 0)
		{
            items_t x(item, std::string());

			std::size_t j = 0;

			while ( j < list.size() && list[j].items < x )
			{
				++j;
			}

			comparison_t y = { x, false, false, false };

			// insert into the list
			list.insert(list.begin() + j, y);

            QStringList items;
            items << qt::convert(item) << QString();
            ui->multilistview->insertItem(j, items);
		}
	}

	// remove empty items
	for ( std::size_t i = 0; i < list.size();)
	{
        if ( list[i].items.right.empty() && list[i].items.left.empty())
		{
            ui->multilistview->removeItem(i);
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
    const std::string & added_or_changed
)
{
    updateRight(std::vector< std::string >(1, added_or_changed));
}

void DirDiffForm::updateRight(
    const std::vector< std::string >& added_or_changed,
    const std::vector< std::string >& remove
)
{
    std::set< std::string > oldright;

	for ( std::size_t i = 0, n = list.size(); i < n; ++i )
	{
        if ( !list[i].items.right.empty())
		{
            if ( pbl::contains(remove, list[i].items.right))
			{
                list[i].items.right = std::string();
				list[i].compared    = false;
				list[i].same        = false;
                ui->multilistview->clearText(1, i);
			}
            else if ( pbl::contains(added_or_changed, list[i].items.right))
			{
				list[i].compared = false;
				list[i].same     = false;
			}

            oldright.insert(list[i].items.right);
		}
	}

    for ( std::size_t i = 0, n = added_or_changed.size(); i < n; ++i )
	{
        const std::string item = added_or_changed[i];

		// was item added?
        if ( oldright.count(item) == 0)
		{
            items_t x(std::string(), item);

			std::size_t j = 0;

			while ( j < list.size() && list[j].items < x )
			{
				++j;
			}

			comparison_t y = { x, false, false, false };

			// insert into the list
			list.insert(list.begin() + j, y);

            QStringList items;
            items << QString() << qt::convert(item);
            ui->multilistview->insertItem(j, items);
		}
	}

	// remove empty items
	for ( std::size_t i = 0; i < list.size();)
	{
        if ( list[i].items.left.empty() && list[i].items.right.empty())
		{
            ui->multilistview->removeItem(i);
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
            ui->multilistview->removeItem(i);
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

            QStringList items;
            items <<  qt::convert(matching[i].left) << qt::convert(matching[i].right);
            ui->multilistview->insertItem(i, items);
		}
	}

	startComparison();
	applyFilters();

}

void DirDiffForm::on_actionSelect_Different_triggered()
{
    QList< int > indices;
    for (std::size_t i = 0, n = list.size(); i < n; ++i)
        if (list[i].compared && !list[i].same)
            indices << i;
    ui->multilistview->setSelectedRows(indices);
}

void DirDiffForm::on_actionSelect_Same_triggered()
{
    QList< int > indices;
    for (std::size_t i = 0, n = list.size(); i < n; ++i)
        if (list[i].compared && list[i].same)
            indices << i;
    ui->multilistview->setSelectedRows(indices);
}

void DirDiffForm::on_actionSelect_Left_Only_triggered()
{
    QList< int > indices;
    for (std::size_t i = 0, n = list.size(); i < n; ++i)
        if (!list[i].items.left.empty() && list[i].items.right.empty())
            indices << i;
    ui->multilistview->setSelectedRows(indices);
}

void DirDiffForm::on_actionSelect_Right_Only_triggered()
{
    QList< int > indices;
    for (std::size_t i = 0, n = list.size(); i < n; ++i)
        if (list[i].items.left.empty() && !list[i].items.right.empty())
            indices << i;
    ui->multilistview->setSelectedRows(indices);
}
