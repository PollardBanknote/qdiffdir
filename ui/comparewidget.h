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
#ifndef COMPAREWIDGET_H
#define COMPAREWIDGET_H

#include <QWidget>
#include <QThread>
#include <QMutex>
#include <QMap>
class QListWidgetItem;

namespace Ui
{
class CompareWidget;
}

/** A pair of strings
 */
struct items_t
{
	items_t(
	    const QString& l,
	    const QString& r
	)
		: left(l), right(r)
	{

	}

	bool operator<(const items_t& o) const
	{
		return ( left.isEmpty() ? right : left ) < ( o.left.isEmpty() ? o.right : o.left );
	}

	bool operator==(const items_t& o) const
	{
		return left == o.left && right == o.right;
	}

	bool operator!=(const items_t& o) const
	{
		return left != o.left || right != o.right;
	}

	QString left;
	QString right;
};

/** Interface for comparing two items based on their names
 */
class Compare
{
public:
	virtual ~Compare()
	{
	}

	virtual bool equal(const QString&, const QString&) = 0;
	virtual Compare* clone() const                     = 0;
};

class WorkerThread : public QThread
{
	Q_OBJECT
public:
	explicit WorkerThread(
	    Compare                     *,
	    const std::vector< items_t >&
	);
	~WorkerThread();

	void clear();
signals:
	void compared(QString, QString, bool);
private:
	typedef std::vector< items_t > TaskList;
	Compare* compare;
	QMutex   lock;
	TaskList todo;
	void run();
};

class Matcher
{
public:
	enum {EXACT_MATCH = 0, DO_NOT_MATCH = 1000};

	virtual ~Matcher()
	{
	}

	virtual Matcher* clone() const = 0;

	// return EXACT_MATCH, DO_NOT_MATCH, or something in between indicating
	// how well the two match (lower is better)
	virtual int compare(const QString&, const QString&) const = 0;
};

/** A widget for comparing items in two lists
 *
 * This widget takes two lists of items, matches items between them, and
 * compares each pair. Ex., the files of two different directories.
 *
 * @todo Rather than accept clone-able base classes, accept as template
 * parameter. Then wrap it with an adapter.
 */
class CompareWidget : public QWidget
{
	Q_OBJECT
public:
	/** Display options
	 */
	enum flags
	{
		ShowRightOnly = 1, ///< Items on the right only will be shown
		ShowLeftOnly  = 2, ///< Items on the left only will be shown
		ShowIdentical = 4  ///< Items that are considered identical will be shown
	};

	/// Constructor
	explicit CompareWidget(QWidget* parent = 0);

	/// Destructor
	~CompareWidget();

	/** Configure which items ar shown
	 * @param f A combination of "flags" values
	 */
	void setFlags(int f);

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

	/** Add a filter to the combo box
	 * @param desc A description of the filter that will appear in the combo box
	 * @param r A regex to apply to the names of items
	 *
	 * Filters control which items are shown or not. Items whose name matches
	 * the regex will be shown.
	 *
	 * @todo Make a Filter class to encapsulate the test. Wrap the regex.
	 */
	void addFilter(const QString& desc, const QRegExp& r);

	/** Get the item selected in the left window
	 */
    QString getCurrentLeft() const;

	/** Get the item selected in the right window
	 */
    QString getCurrentRight() const;

    QStringList getSelectedLeft() const;
    QStringList getSelectedRight() const;

	void setLeftAndRight(const QString& leftname, const QString& rightname, const QStringList& leftitems, const QStringList& rightitems);
	void updateLeft(const QStringList& added_or_changed, const QStringList& remove = QStringList());
	void updateRight(const QStringList& added_or_changed, const QStringList& remove = QStringList());
signals:
	/** Notify connected objects that the user double clicked an item
	 * @param l The left item
	 * @param r The right item
	 */
	void itemDoubleClicked(QString l, QString r);
private slots:
	void applyFilters();

	/** The user has changed the filter
	 */
	void on_filter_activated(int);

	/** Scroll the left and right lists to the same point
	 */
	void sync_scroll(int);

	/** Change the scroll bar range to match the list widgets
	 */
	void setScrollBarRange(int, int);

	/** Responds to QListWidget::itemDoubleClicked
	 */
	void do_action(QListWidgetItem*);

	/** Mark an item as ignored
	 */
	void on_actionIgnore_triggered();

	/** Copy a summary to the clipboard
	 *
	 * The summary will be something like "Left item\tRight Item\tSame\n"
	 */
	void on_actionCopy_To_Clipboard_triggered();

	/** The "show left only" checkbox was toggled
	 */
	void on_showonlyleft_toggled(bool checked);

	/** The "show ignored" checkbox was toggled
	 */
	void on_showIgnored_toggled(bool checked);

	/** The "show identical items" checkbox was toggled
	 */
	void on_showsame_toggled(bool checked);

	/** The "show right only" checkbox was toggled
	 */
	void on_showonlyright_toggled(bool checked);

	/** Change the currently selected item in both lists. Scroll to it
	 */
	void changesel(int);

    void copy_selection_to_left();
    void copy_selection_to_right();

	/** Respond to the worker when it has finished comparing two items
	 * @param l The identifier of the left item
	 * @param r The identifier of the right item
	 * @param same True iff items compared "the same"
	 */
	void items_compared(QString l, QString r, bool same);
private:
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

	/** Interrupt the worker thread and stop doing any more comparisons
	 */
	void stopComparison();

	/** Start a worker thread for comparing matched items
	 */
	void startComparison();

	/** Setup signals for keeping the two list widgets scrolled to the same point
	 */
	void syncWindows();

	/** Stop the synchronization between the list widgets
	 *
	 * This function is necessary so that the GUI doesn't go wild as many
	 * changes are made to the lists. It seems like a bit of a workaround.
	 */
	void unsyncWindows();

	/** Deselect all items
	 */
	void clearSelection();

	/** Match left and right items, restart the comparisons
	 */
	void rematch();

	/** Check if an item should be hidden, according to current view options
	 */
	bool hidden(std::size_t) const;

	/** Apply styling to a list item so we can quickly see the comparison
	 *
	 * Ex., red for "not the same"; grey for "not yet compared"; green for
	 * "doesn't have a match"; strike through for "ignore it"
	 */
	void style(QListWidgetItem* item, bool hidden, std::size_t i) const;

	/// Pointer to UI class c/o Qt Creator
	Ui::CompareWidget* ui;

	/// Object that does the comparison of items
	Compare* compare;

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

	/// A thread that performs comparisons in the background
	WorkerThread* worker;

	/// Object that matches item names between left and right lists
	Matcher* matcher;
};

#endif // COMPAREWIDGET_H
