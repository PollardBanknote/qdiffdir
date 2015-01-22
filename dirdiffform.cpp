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

#include "fileutils/qutils.h"
#include "fileutils/fileutils.h"

namespace
{
// Check that two files are equal. Files are relative to the directories passed
// to the constructor
class FileCompare : public Compare
{
public:
	FileCompare(const QDir& l, const QDir& r)
		: left(l), right(r)
	{

	}

	FileCompare* clone() const
	{
		return new FileCompare(*this);
	}

	/// @todo Don't unzip a file if we don't need to
	/// @todo Don't read entire files into memory when doing the file compare
	/// @todo Don't run cmp, write our own function
	bool equal(
		const QString& lfile,
		const QString& rfile
	)
	{
		const QString first  = left.absoluteFilePath(lfile);
		const QString second = right.absoluteFilePath(rfile);

		bool diff;

		if ( first.endsWith(".gz") || second.endsWith(".gz"))
		{
			const QByteArray data1 = gunzip(first);
			const QByteArray data2 = gunzip(second);

			diff = ( data1 != data2 );
		}
		else
		{
			if (QFileInfo(first).size() != QFileInfo(second).size())
			{
				return false;
			}
			else
			{
				QStringList l;

				l << first << second;

				diff = ( QProcess::execute("/usr/bin/cmp", l) != 0 );
			}
		}

		return !diff;
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

	QDir left;
	QDir right;

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

DirDiffForm::DirDiffForm(QWidget* parent_) :
	QWidget(parent_),
	ui(new Ui::DirDiffForm), leftdir(), rightdir(), watcher()
{
	ui->setupUi(this);
	ui->compareview->setMatcher(FileNameMatcher());
	ui->compareview->addFilter("Source Files (*.cpp, *.h)", QRegExp(".*(cpp|h)"));
	connect(ui->compareview, SIGNAL(itemDoubleClicked(QString, QString)), SLOT(viewfiles(QString, QString)));
	changeDirectories(leftdir.absolutePath(), rightdir.absolutePath());
}

DirDiffForm::~DirDiffForm()
{
	delete ui;
}

void DirDiffForm::setFlags(bool show_left_only, bool show_right_only, bool show_identical)
{
	const int f = (show_left_only ? CompareWidget::ShowLeftOnly : 0)
		| (show_right_only ? CompareWidget::ShowRightOnly : 0)
		| (show_identical ? CompareWidget::ShowIdentical : 0);

	ui->compareview->setFlags(f);
}

void DirDiffForm::on_viewdiff_clicked()
{
	QString s1 = ui->compareview->getSelectedLeft();
	QString s2 = ui->compareview->getSelectedRight();

	if ( s1.isEmpty() && s2.isEmpty())
	{
		QMessageBox::warning(this, "No file selected", "Cannot complete action");
		return;
	}

	if ( s1.isEmpty()) // view s2
	{
		QProcess::startDetached("gvim", QStringList(s2), rightdir.absolutePath());
	}
	else if ( s2.isEmpty()) // view s1
	{
		QProcess::startDetached("gvim", QStringList(s1), leftdir.absolutePath());
	}
	else // run gvimdiff
	{
		QStringList l;
		l << leftdir.absoluteFilePath(s1)
		  << rightdir.absoluteFilePath(s2);
		QProcess::startDetached("gvimdiff", l);
	}
}

// s is relative to source
void DirDiffForm::copyTo(
	const QString& s,
	const QDir&    source,
	const QDir&    destination
)
{
	if ( s.isEmpty())
	{
		QMessageBox::warning(this, "No file selected", "Cannot complete action");
		return;
	}

	QString abspath = source.absoluteFilePath(s);

	copyFile(abspath, destination, s);
}

void DirDiffForm::saveAs(
	const QString& original_filename,
	const QDir&    source,
	const QDir&    destination
)
{
	if ( !original_filename.isEmpty())
	{
		QStringList suggested_relative_path = original_filename.split(QDir::separator());
		suggested_relative_path.pop_back(); // remove the filename

		QDir suggested_save_directory = destination;

		bool valid_dir = true;

		for ( int i = 0; valid_dir == true && i < suggested_relative_path.size(); ++i )
		{
			valid_dir = suggested_save_directory.cd(suggested_relative_path[i]);
		}

		if ( !valid_dir )
		{
			suggested_save_directory = destination;
		}

		QString suggested_filename = suggested_save_directory.absolutePath() + QDir::separator() + lastPathComponent(original_filename);
		QString save_file_name     = QFileDialog::getSaveFileName(this, "Save as", suggested_filename);

		if ( !save_file_name.isEmpty())
		{
			copyFile(source.absoluteFilePath(original_filename), destination, destination.relativeFilePath(save_file_name));
		}
	}
}

void DirDiffForm::on_copytoright_clicked()
{
	copyTo(ui->compareview->getSelectedLeft(), leftdir, rightdir);
}

void DirDiffForm::on_copytoleft_clicked()
{
	copyTo(ui->compareview->getSelectedRight(), rightdir, leftdir);
}

void DirDiffForm::on_renametoright_clicked()
{
	saveAs(ui->compareview->getSelectedLeft(), leftdir, rightdir);
}

void DirDiffForm::on_renametoleft_clicked()
{
	saveAs(ui->compareview->getSelectedRight(), rightdir, leftdir);
}

void DirDiffForm::on_openleftdir_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, "Choose a directory", leftdir.absolutePath());

	if ( !s.isEmpty())
	{
		changeDirectories(s, QString());
	}
}

void DirDiffForm::on_openrightdir_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, "Choose a directory", rightdir.absolutePath());

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
			QStringList(rightdir.absoluteFilePath(s2)));
	}
	else if ( s2.isEmpty())
	{
		QProcess::startDetached("gvim",
			QStringList(leftdir.absoluteFilePath(s1)));
	}
	else // run gvimdiff
	{
		QStringList l;
		l << leftdir.absoluteFilePath(s1)
		  << rightdir.absoluteFilePath(s2);
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

void DirDiffForm::on_depth_valueChanged(int d)
{
	{
		QStringList files = getRecursiveRelativeFilenames(leftdir, d);
		QStringList rem   = ui->compareview->getAllLeft();

		for ( int i = 0; i < files.count(); ++i )
		{
			rem.removeAll(files.at(i));
		}

		ui->compareview->updateLeft(files, rem);
	}
	{
		QStringList files = getRecursiveRelativeFilenames(rightdir, d);
		QStringList rem   = ui->compareview->getAllRight();

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
		leftdir.cd(left);
		ui->openleftdir->setText(lastPathComponent(dirName(leftdir)));
	}

	if ( !right.isEmpty())
	{
		rightdir.cd(right);
		ui->openrightdir->setText(lastPathComponent(dirName(rightdir)));
	}

	// refresh the compareview
	resetView();

	// create new file system watcher
	// Note: there is a bug that causes a crash if QFileSystemWatcher gets the
	// same path twice
	QStringList dirlist;
	dirlist << getRecursiveDirectories(leftdir, ui->depth->value())
	        << getRecursiveDirectories(rightdir, ui->depth->value());

	dirlist.removeDuplicates();

	watcher = new QFileSystemWatcher(dirlist, this);
	connect(watcher, SIGNAL(directoryChanged(QString)), SLOT(contentsChanged(QString)));
}

void DirDiffForm::copyFile(
	const QString& file,
	const QDir&    dest,
	const QString& newname
)
{
	QString destpath = dest.absoluteFilePath(newname);

	if ( QFile::exists(destpath))
	{
		if ( QMessageBox::question(this, "File exists", "Do you want to overwrite the destination?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::No )
		{
			return;
		}
	}

	if ( pbl::file::copy(file.toStdString(), destpath.toStdString()))
	{
		fileChanged(destpath);
	}
}

// File system has notified us of a change in one of our directories
/// @bug Watch any new subdirectories
/// @todo added_or_changed should not include files that haven't changed
void DirDiffForm::contentsChanged(QString dirname_)
{
	QDir          eventdir(dirname_);
	const QString dirname = dirName(eventdir); // absolute path of dir

	// absolute path of every file below dirname
	if ( dirname.startsWith(dirName(leftdir)))
	{
		// Current list of files in dirname
		QStringList curr;

		const int depth_ = ui->depth->value()
		                   - (( dirname.count(QDir::separator()) - dirName(leftdir).count(QDir::separator())) - 1 );

		if ( depth_ > 0 )
		{
			curr = getRecursiveAbsoluteFilenames(dirname, depth_ - 1);
		}

		QStringList added_or_changed;
		QStringList removed;

		const QStringList old = ui->compareview->getAllLeft();

		// for each file in old list: check if changed or removed
		for ( int i = 0; i < old.count(); ++i )
		{
			const QString x = leftdir.absoluteFilePath(old.at(i));

			if ( x.startsWith(dirname))
			{
				if ( curr.contains(x))
				{
					QFileInfo fi(x);

					// changed
					if ( fi.lastModified() > when )
					{
						added_or_changed << old.at(i);
					}
				}
				else
				{
					removed << old.at(i);
				}
			}
		}

		// for each file in new list: check if added
		for ( int i = 0; i < curr.count(); ++i )
		{
			const QString x = leftdir.relativeFilePath(curr.at(i));

			if ( !old.contains(x))
			{
				added_or_changed << x;
			}
		}

		ui->compareview->updateLeft(added_or_changed, removed);
	}

	if ( dirname.startsWith(dirName(rightdir)))
	{
		QStringList curr;

		const int depth_ = ui->depth->value()
		                   - (( dirname.count(QDir::separator()) - dirName(rightdir).count(QDir::separator())) - 1 );

		if ( depth_ > 0 )
		{
			curr = getRecursiveAbsoluteFilenames(dirname, depth_ - 1);
		}

		QStringList       added_or_changed;
		QStringList       removed;
		const QStringList old = ui->compareview->getAllRight();

		// for each file in old list: check if changed or removed
		for ( int i = 0; i < old.count(); ++i )
		{
			const QString x = rightdir.absoluteFilePath(old.at(i));

			if ( x.startsWith(dirname))
			{
				if ( curr.contains(x))
				{
					// changed
					QFileInfo fi(x);

					if ( fi.lastModified() > when )
					{
						added_or_changed << old.at(i);
					}
				}
				else
				{
					removed << old.at(i);
				}
			}
		}

		// for each file in new list: check if added
		for ( int i = 0; i < curr.count(); ++i )
		{
			const QString x = rightdir.relativeFilePath(curr.at(i));

			if ( !old.contains(x))
			{
				added_or_changed << x;
			}
		}

		ui->compareview->updateRight(added_or_changed, removed);
	}
}

// A single file has been changed/or added. Everything else stayed the same
// file is an absolute path
void DirDiffForm::fileChanged(QString file)
{
	if ( file.startsWith(leftdir.absolutePath()))
	{
		ui->compareview->updateLeft(QStringList(leftdir.relativeFilePath(file)));
	}

	if ( file.startsWith(rightdir.absolutePath()))
	{
		ui->compareview->updateRight(QStringList(rightdir.relativeFilePath(file)));
	}
}

// one or both directories changed -- redo the whole thing
void DirDiffForm::resetView()
{
	const int   depth_     = ui->depth->value();
	QStringList leftfiles  = getRecursiveRelativeFilenames(leftdir, depth_);
	QStringList rightfiles = getRecursiveRelativeFilenames(rightdir, depth_);

	when = QDateTime::currentDateTime();

	ui->compareview->setLeftAndRight(dirName(leftdir), dirName(rightdir), leftfiles, rightfiles);
	ui->compareview->setComparison(FileCompare(leftdir, rightdir));
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
	changeDirectories(leftdir.absolutePath(), rightdir.absolutePath());
}

void DirDiffForm::on_swap_clicked()
{
	changeDirectories(rightdir.absolutePath(), leftdir.absolutePath());
}
