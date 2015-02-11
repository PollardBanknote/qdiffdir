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

#include <QMessageBox>
#include <QProcess>
#include <QFileDialog>
#include <QInputDialog>
#include <QDebug>
#include <QFileSystemWatcher>

#include "fs/fileutils.h"
#include "fs/diriter.h"

namespace
{
// Check that two files are equal. Files are relative to the directories passed
// to the constructor
class FileCompare : public Compare
{
public:
	FileCompare(
		const QString& l,
		const QString& r
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
			return pbl::fs::compare(first.toStdString(), second.toStdString()) == 1;
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
	ui(new Ui::DirDiffForm), watcher()
{
	ui->setupUi(this);
	ui->compareview->setMatcher(FileNameMatcher());
	ui->compareview->addFilter("Source Files (*.cpp, *.h)", QRegExp(".*(cpp|h)"));
	connect(ui->compareview, SIGNAL(itemDoubleClicked(QString, QString)), SLOT(viewfiles(QString, QString)));
	changeDirectories(ldir.absolutePath(), rdir.absolutePath());
}

DirDiffForm::~DirDiffForm()
{
	delete ui;
}

void DirDiffForm::setFlags(
	bool show_left_only,
	bool show_right_only,
	bool show_identical
)
{
	const int f = ( show_left_only ? CompareWidget::ShowLeftOnly : 0 )
				  | ( show_right_only ? CompareWidget::ShowRightOnly : 0 )
				  | ( show_identical ? CompareWidget::ShowIdentical : 0 );

	ui->compareview->setFlags(f);
}

void DirDiffForm::on_viewdiff_clicked()
{
	const QString s1 = ui->compareview->getSelectedLeft();
	const QString s2 = ui->compareview->getSelectedRight();

	if ( s1.isEmpty() && s2.isEmpty())
	{
		QMessageBox::warning(this, "No file selected", "Cannot complete action");
		return;
	}

	if ( s1.isEmpty()) // view s2
	{
		QProcess::startDetached("gvim", QStringList(s2), rdir.absolutePath());
	}
	else if ( s2.isEmpty()) // view s1
	{
		QProcess::startDetached("gvim", QStringList(s1), ldir.absolutePath());
	}
	else // run gvimdiff
	{
		QStringList l;
		l << ldir.absoluteFilePath(s1)
		  << rdir.absoluteFilePath(s2);
		QProcess::startDetached("gvimdiff", l);
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
	const QString s = ui->compareview->getSelectedLeft();

	if ( s.isEmpty())
	{
		QMessageBox::warning(this, "No file selected", "Cannot complete action");
		return;
	}

	copyTo(ldir.absoluteFilePath(s), directoryComponent(rdir.absolutePath() + "/" + s));
}

void DirDiffForm::on_copytoleft_clicked()
{
	const QString s = ui->compareview->getSelectedRight();

	if ( s.isEmpty())
	{
		QMessageBox::warning(this, "No file selected", "Cannot complete action");
		return;
	}

	copyTo(rdir.absoluteFilePath(s), directoryComponent(ldir.absolutePath() + "/" + s));
}

void DirDiffForm::on_renametoright_clicked()
{
	saveAs(ui->compareview->getSelectedLeft(), ldir.absolutePath(), rdir.absolutePath());
}

void DirDiffForm::on_renametoleft_clicked()
{
	saveAs(ui->compareview->getSelectedRight(), rdir.absolutePath(), ldir.absolutePath());
}

void DirDiffForm::on_openleftdir_clicked()
{
	const QString s = QFileDialog::getExistingDirectory(this, "Choose a directory", ldir.absolutePath());

	if ( !s.isEmpty())
	{
		changeDirectories(s, QString());
	}
}

void DirDiffForm::on_openrightdir_clicked()
{
	const QString s = QFileDialog::getExistingDirectory(this, "Choose a directory", rdir.absolutePath());

	if ( !s.isEmpty())
	{
		changeDirectories(QString(), s);
	}
}

void DirDiffForm::viewfiles(
	QString s1,
	QString s2
)
{
	if ( s1.isEmpty())
	{
		QProcess::startDetached("gvim",
			QStringList(rdir.absoluteFilePath(s2)));
	}
	else if ( s2.isEmpty())
	{
		QProcess::startDetached("gvim",
			QStringList(ldir.absoluteFilePath(s1)));
	}
	else // run gvimdiff
	{
		QStringList l;
		l << ldir.absoluteFilePath(s1)
		  << rdir.absoluteFilePath(s2);
		QProcess::startDetached("gvimdiff", l);
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
		int     n  = QInputDialog::getInteger(this, "Renumber", "Please enter the new file number", x_, 1, INT_MAX, 1, &ok);

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
	{
		QStringList       rem   = ldir.getRelativeFileNames();
		const QStringList files = ldir.setDepth(d);

		for ( int i = 0; i < files.count(); ++i )
		{
			rem.removeAll(files.at(i));
		}

		ui->compareview->updateLeft(files, rem);
	}
	{
		QStringList       rem   = rdir.getRelativeFileNames();
		const QStringList files = rdir.setDepth(d);

		for ( int i = 0; i < files.count(); ++i )
		{
			rem.removeAll(files.at(i));
		}

		ui->compareview->updateRight(files, rem);
	}
}

void DirDiffForm::changeDirectories(
	const QString& left,
	const QString& right
)
{
	// stop the watcher, and clear the comparisons we know
	delete watcher;

	// change to the new directories
	if ( !left.isEmpty())
	{
		ldir.cd(left);
		ui->openleftdir->setText(ldir.name());
	}

	if ( !right.isEmpty())
	{
		rdir.cd(right);
		ui->openrightdir->setText(rdir.name());
	}

	const int         depth_     = ui->depth->value();
	const QStringList leftfiles  = ldir.setDepth(depth_);
	const QStringList rightfiles = rdir.setDepth(depth_);

	{
		/// @todo If left and right are the same, don't do both
		/// @todo If a dir hasn't changed, don't go through it
		// refresh the compareview

		when = QDateTime::currentDateTime();

		ui->compareview->setLeftAndRight(ldir.absolutePath(), rdir.absolutePath(), leftfiles, rightfiles);
		ui->compareview->setComparison(FileCompare(ldir.absolutePath(), rdir.absolutePath()));
	}

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
		if ( QMessageBox::question(this, "File exists", "Do you want to overwrite the destination?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::No )
		{
			return;
		}
	}

	if ( pbl::fs::copy(file.toStdString(), s.toStdString()))
	{
		fileChanged(s);
	}
}

// File system has notified us of a change in one of our directories
/// @bug Watch any new subdirectories that popped up
/// @todo added_or_changed should not include files that haven't changed
void DirDiffForm::contentsChanged(QString dirname_)
{
	QDir          eventdir(dirname_);
	const QString dirname = dirName(eventdir); // absolute path of dir

	// absolute path of every file below dirname
	if ( dirname.startsWith(ldir.absolutePath()))
	{
		DirectoryContents::update_t u = ldir.update(dirname);

		for ( int i = 0, n = u.changed.count(); i < n; ++i )
		{
			QFileInfo fi(ldir.absoluteFilePath(u.changed.at(i)));

			if ( fi.lastModified() > when )
			{
				u.added << u.changed.at(i);
			}
		}

		ui->compareview->updateLeft(u.added, u.removed);
	}

	if ( dirname.startsWith(rdir.absolutePath()))
	{
		DirectoryContents::update_t u = rdir.update(dirname);

		for ( int i = 0, n = u.changed.count(); i < n; ++i )
		{
			QFileInfo fi(rdir.absoluteFilePath(u.changed.at(i)));

			if ( fi.lastModified() > when )
			{
				u.added << u.changed.at(i);
			}
		}

		ui->compareview->updateRight(u.added, u.removed);
	}
}

// A single file has been changed/or added. Everything else stayed the same
// file is an absolute path
void DirDiffForm::fileChanged(QString file)
{
	if ( file.startsWith(ldir.absolutePath()))
	{
		ui->compareview->updateLeft(QStringList(ldir.relativeFilePath(file)));
	}

	if ( file.startsWith(rdir.absolutePath()))
	{
		ui->compareview->updateRight(QStringList(rdir.relativeFilePath(file)));
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
	changeDirectories(ldir.absolutePath(), rdir.absolutePath());
}

void DirDiffForm::on_swap_clicked()
{
	/// @todo does a lot of work unnecessarily
	changeDirectories(rdir.absolutePath(), ldir.absolutePath());
}
