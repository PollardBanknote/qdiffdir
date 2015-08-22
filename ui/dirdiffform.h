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
#include <QThread>
#include <QMutex>

class QDir;
class QListWidgetItem;
class QString;
class QFileSystemWatcher;

#include "directorycontents.h"
#include "directorycomparison.h"

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

	/** Set the object used to match items between lists
	 *
	 * By default, items are matched if they have the exact same name. A
	 * matcher can override or extend that behavior. See the documentation for
	 * that class.
	 */
	void setMatcher(const Matcher&);

	/** Set the object used to compare matched items
	 *
	 * After items are matched, they are compared and the result is shown in
	 * the widget. This function allows one to control how items are considered
	 * to be "the same". By default, no items are considerd "the same".
	 */
	void setComparison(const Compare&);

	void setLeftAndRight(const QString& leftname, const QString& rightname, const QStringList& leftitems, const QStringList& rightitems);
	void updateLeft(const QStringList& added_or_changed, const QStringList& remove = QStringList());
	void updateRight(const QStringList& added_or_changed, const QStringList& remove = QStringList());
public slots:
	/** The "show left only" checkbox was toggled
	 */
	void showOnlyLeft(bool checked);

	/** The "show ignored" checkbox was toggled
	 */
	void showIgnored(bool checked);

	/** The "show identical items" checkbox was toggled
	 */
	void showSame(bool checked);

	/** The "show right only" checkbox was toggled
	 */
	void showOnlyRight(bool checked);

	void setFilter(const QRegExp&);

	void clearFilter();
signals:
	/** Notify connected objects that the user double clicked an item
	 * @param l The left item
	 * @param r The right item
	 */
	void itemDoubleClicked(QString l, QString r);
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

	void on_filter_editTextChanged(const QString& arg1);

	void on_autoRefresh_stateChanged(int state);

	void applyFilters();

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
	void items_compared(QString l, QString r, bool same);
    void on_actionSelect_Different_triggered();

    void on_actionSelect_Same_triggered();

    void on_actionSelect_Left_Only_triggered();

    void on_actionSelect_Right_Only_triggered();

private:
    enum overwrite_t { OVERWRITE_ASK, OVERWRITE_YES, OVERWRITE_NO };

	void saveAs(const QString&, const QString& source, const QString& destination);
	void saveAs(const QStringList&, const QString&, const QString&);
    std::pair< bool, overwrite_t> copyTo(const QString& file, const QString& destdir, const QString& newname, overwrite_t);
	void stopDirectoryWatcher();
	void startDirectoryWatcher();
	void fileChanged(QString);
    void filesChanged(const QStringList&);
	QString renumber(const QString& s_);
	QString dirName(const QDir& dir);
	QString getDirectory(const QString& dir);

	struct comparison_t
	{
		items_t items;
		bool compared;
		bool same;
		bool ignore;

		bool left_only() const
		{
			return !items.left.isEmpty() && items.right.isEmpty();
		}

		bool right_only() const
		{
			return items.left.isEmpty() && !items.right.isEmpty();
		}

		bool unmatched() const
		{
			return items.left.isEmpty() || items.right.isEmpty();
		}

	};

	/// Get a list of all items on the left
	QStringList getAllLeft() const;

	/// Get a list of all items on the right
	QStringList getAllRight() const;

	/** Perform a matching on the two lists of items
	 * @param l Items on the left
	 * @param r Items on the right
	 */
	std::vector< items_t > match(const QStringList& l, const QStringList& r) const;

	/** Start a worker thread for comparing matched items
	 */
	void startComparison();

	/** Match left and right items, restart the comparisons
	 */
	void rematch();

	/** Check if an item should be hidden, according to current view options
	 */
	bool hidden(std::size_t) const;

	/// Pointer to UI class c/o Qt Creator
	Ui::DirDiffForm* ui;

	/// A filter for which items to show
	QRegExp filter;

	QString leftname;
	QString rightname;

	/// Whether or not to show left only items
	bool hide_left_only;

	/// Whether or not to show right only items
	bool hide_right_only;

	/// Whether or not to show identical items
	bool hide_identical_items;

	/// Whether or not to show items marked as ignored
	bool hide_ignored;

	std::vector< comparison_t > list;

	DirectoryComparison derp;
	QDateTime           when;                  // last time directories were updated
	QFileSystemWatcher* watcher;
};

#endif // DIRDIFFFORM_H
