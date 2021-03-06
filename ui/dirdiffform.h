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

#include <set>

#include <QMap>
#include <QWidget>
#include <QDateTime>
#include <QThread>
#include <QMutex>

class QDir;
class QListWidgetItem;
class QString;
class QFileSystemWatcher;

#include "filecompare.h"
#include "comparisonlist.h"
#include "pbl/fileutil/directorycontents.h"

namespace Ui
{
class DirDiffForm;
}

/** A widget for comparing two directories, with some file operations
 *
 */
class DirDiffForm
	: public QWidget
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
	void changeDirectories(const std::string& left, const std::string& right);

	/** Change view options
	 * @param show_left_only Show files that appear on the left only
	 * @param show_right_only Show files that appear on the right only
	 * @param show_identical Show files that are equivalent on the left and right
	 */
	void setFlags(bool show_left_only, bool show_right_only, bool show_identical);
public slots:
	void settingsChanged();
signals:
	void compare_files(const QString&, const QString&, const QString&, const QString&, int);
private slots:
	void on_viewdiff_clicked();
	void on_copytoright_clicked();
	void on_copytoleft_clicked();
	void on_renametoright_clicked();
	void on_renametoleft_clicked();
	void on_openleftdir_clicked();
	void on_openrightdir_clicked();
	void on_depth_valueChanged(int arg1);
	void viewfiles(int);
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

	void on_autoRefresh_stateChanged(int state);

	/** Mark an item as ignored
	 */
	void on_actionIgnore_triggered();

	/** Copy a summary to the clipboard
	 *
	 * The summary will be something like "Left item\tRight Item\tSame\n"
	 */
	void on_actionCopy_To_Clipboard_triggered();

	/** Respond to the worker when it has finished comparing two items
	 * @param l The identifier of the left item
	 * @param r The identifier of the right item
	 * @param same True iff items compared "the same"
	 */
	void items_compared(const QString& l, const QString& r, bool same);
	void on_actionSelect_Different_triggered();

	void on_actionSelect_Same_triggered();

	void on_actionSelect_Left_Only_triggered();

	void on_actionSelect_Right_Only_triggered();
	void on_depthlimit_toggled(bool checked);
private:
	enum overwrite_t {OVERWRITE_ASK, OVERWRITE_YES, OVERWRITE_NO};

	std::vector< std::string > get_section_files(std::size_t);

	void copyfiles(std::size_t, std::size_t);

	void populate_filters();

	void applyFilters();

	void show_only_section(std::size_t, bool checked);

	/** The "show ignored" checkbox was toggled
	 */
	void showIgnored(bool checked);

	/** The "show identical items" checkbox was toggled
	 */
	void showSame(bool checked);

	void setFilter(const QRegExp&);

	void setFilters(const QString&);

	void saveAs(std::size_t, std::size_t);
	std::pair< bool, overwrite_t > copyTo(const std::string & file, const std::string&, overwrite_t);
	void stopDirectoryWatcher();
	void startDirectoryWatcher();
	void filesChanged(const std::set< std::string >&);
	std::string getDirectory(const std::string& dir);
	void change_depth();
	void open_section(std::size_t);
	void change_dir(const std::string&, const std::string&);
	void find_subdirs(QStringList& subdirs, const DirectoryContents& n, const std::string&, int, int);
	QStringList find_subdirs(const DirectoryContents&, int);
	void refresh();
	int get_depth();
	void explore_section(std::size_t);
	void select_section_only(std::size_t);

	void file_list_changed(int depth, bool);

	/** Start a worker thread for comparing matched items
	 */
	void startComparison();

	/** Check if an item should be hidden, according to current view options
	 */
	bool hidden(std::size_t) const;

	/// Pointer to UI class c/o Qt Creator
	Ui::DirDiffForm* ui;

	QThread compare_thread;

	/// A filter for which items to show
	QVector< QRegExp > filters;

	/// Whether or not to show left only items
	bool hide_section_only[2];

	/// Whether or not to show identical items
	bool hide_identical_items;

	/// Whether or not to show items marked as ignored
	bool hide_ignored;

	QString section_name[2];

	DirectoryContents section_tree[2];

	std::vector< comparison_t > list;
	/*
	   DirectoryComparison derp;
	 */
	QDateTime           when;                  // last time directories were updated
	QFileSystemWatcher* watcher;
	QStringList         watched_dirs;
};

#endif // DIRDIFFFORM_H
