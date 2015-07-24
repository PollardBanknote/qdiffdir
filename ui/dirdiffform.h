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
#ifndef DIRDIFFFORM_H
#define DIRDIFFFORM_H

#include <QMap>
#include <QWidget>
#include <QDateTime>

class QDir;
class QListWidgetItem;
class QString;
class QFileSystemWatcher;

#include "directorycontents.h"

namespace Ui
{
class DirDiffForm;
}

/** A widget for comparing two directories, with some file operations
 *
 */
class DirDiffForm : public QWidget
{
	Q_OBJECT
public:
	/** Create the widget with both views set to the current directory
	 */
	explicit DirDiffForm(QWidget* parent);

	/** Destroy the widget and free resources
	 */
	~DirDiffForm();

	/** Change the current directories
	 * @param left Path for the left directory
	 * @param right Path for the right directory
	 */
	void changeDirectories(const QString& left, const QString& right);

	/** Change view options
	 * @param show_left_only Show files that appear on the left only
	 * @param show_right_only Show files that appear on the right only
	 * @param show_identical Show files that are equivalent on the left and right
	 */
	void setFlags(bool show_left_only, bool show_right_only, bool show_identical);
private slots:
	void on_viewdiff_clicked();
	void on_copytoright_clicked();
	void on_copytoleft_clicked();
	void on_renametoright_clicked();
	void on_renametoleft_clicked();
	void on_openleftdir_clicked();
	void on_openrightdir_clicked();
	void on_depth_valueChanged(int arg1);
	void viewfiles(QString, QString);
	void on_refresh_clicked();
	void on_swap_clicked();
	void contentsChanged(QString);
    void on_openright_clicked();

    void on_openleft_clicked();

    void on_showleftonly_toggled(bool checked);

    void on_showrightonly_toggled(bool checked);

    void on_showignored_toggled(bool checked);

    void on_showsame_toggled(bool checked);

    void on_filter_activated(int index);

    void on_filter_editTextChanged(const QString &arg1);

private:
	void saveAs(const QString&, const QString& source, const QString& destination);
    void saveAs(const QStringList&, const QString&, const QString&);
	void copyTo(const QString& file, const QString& destdir);
	void copyTo(const QString& file, const QString& destdir, const QString& newname);

	void fileChanged(QString);
	QString renumber(const QString& s_);
	QString dirName(const QDir& dir);
    QString getDirectory(const QString& dir);

	Ui::DirDiffForm*    ui;
	DirectoryContents   ldir;
	DirectoryContents   rdir;
	QDateTime           when;                  // last time directories were updated
	QFileSystemWatcher* watcher;
};

#endif // DIRDIFFFORM_H
