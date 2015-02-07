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
 */
class CompareWidget : public QWidget
{
	Q_OBJECT
public:
    /** Display options
     */
    enum flags {
        ShowRightOnly = 1, ///< Items on the right only will be shown
        ShowLeftOnly = 2,  ///< Items on the left only will be shown
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

    /** Get a list of items that are selected in the left window
     */
	QString getSelectedLeft() const;

    /** Get a list of items that are selected in the right window
     */
	QString getSelectedRight() const;
	void setLeftAndRight(const QString& leftname, const QString& rightname, const QStringList& leftitems, const QStringList& rightitems);
	void updateLeft(const QStringList& added_or_changed, const QStringList& remove = QStringList());
	void updateRight(const QStringList& added_or_changed, const QStringList& remove = QStringList());
signals:
	void itemDoubleClicked(QString, QString);
private slots:
	void applyFilters();
	void on_filter_activated(int);
	void sync_scroll(int);
	void setScrollBarRange(int, int);
	void do_action(QListWidgetItem*);
	void on_actionIgnore_triggered();
	void on_actionCopy_To_Clipboard_triggered();
	void on_showonlyleft_toggled(bool checked);
	void on_showIgnored_toggled(bool checked);
	void on_showsame_toggled(bool checked);
	void on_showonlyright_toggled(bool checked);
	void changesel(int);
	void items_compared(QString, QString, bool);
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

	QStringList getAllLeft() const;
	QStringList getAllRight() const;
	std::vector< items_t > match(const QStringList&, const QStringList&) const;
	void stopComparison();
	void startComparison();
	void syncWindows();
	void unsyncWindows();
	void clearSelection();
	void rematch();
	void changesel(const QString& leftsel, const QString& rightsel);
	bool hidden(std::size_t) const;
	void style(QListWidgetItem* item, bool hidden, std::size_t i) const;

	Ui::CompareWidget* ui;
	Compare*           compare;
	QRegExp            filter; // item names must pass the filter to be shown
	QString            leftname;
	QString            rightname;
	bool               hide_left_only;
	bool               hide_right_only;
	bool               hide_identical_files;
	bool               hide_ignored;

	std::vector< comparison_t > list;
	WorkerThread*               worker;
	Matcher*                    matcher;
};

#endif // COMPAREWIDGET_H
