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
/** @todo Replace references to "file" with "item"
 * @todo Only do compares for non-filtered items
 */
#include "comparewidget.h"
#include "ui_comparewidget.h"

#include <iostream>

#include <QTextStream>
#include <QClipboard>

#include <QProcess>
#include <QFileInfo>

class DefaultMatcher : public Matcher
{
public:
	DefaultMatcher* clone() const
	{
		return new DefaultMatcher;
	}

	int compare(
		const QString& a,
		const QString& b
	) const
	{
		return a == b ? EXACT_MATCH : DO_NOT_MATCH;
	}

};

const std::size_t NOT_FOUND = std::size_t(-1);

CompareWidget::CompareWidget(QWidget* parent_) :
	QWidget(parent_),
    ui(new Ui::CompareWidget), compare(0), filter(),
	hide_left_only(false),
	hide_right_only(false), hide_identical_items(false), hide_ignored(false),
	worker(0), matcher(new DefaultMatcher)
{
	ui->setupUi(this);

	ui->leftdir->addAction(ui->actionIgnore);
	ui->leftdir->addAction(ui->actionCopy_To_Clipboard);

	ui->rightdir->addAction(ui->actionIgnore);
	ui->rightdir->addAction(ui->actionCopy_To_Clipboard);

	syncWindows();
}

CompareWidget::~CompareWidget()
{
	stopComparison();
	delete compare;
	delete matcher;
	delete ui;
}

void CompareWidget::setMatcher(const Matcher& m)
{
	if ( Matcher * p = m.clone())
	{
		delete matcher;
		matcher = p;
	}
}

void CompareWidget::stopComparison()
{
	delete worker;
	worker = 0;
}

void CompareWidget::syncWindows()
{
	ui->verticalScrollBar->setRange(ui->leftdir->verticalScrollBar()->minimum(), ui->leftdir->verticalScrollBar()->maximum());
	connect(ui->leftdir->verticalScrollBar(), SIGNAL(rangeChanged(int, int)), SLOT(setScrollBarRange(int, int)));
	connect(ui->verticalScrollBar, SIGNAL(valueChanged(int)), SLOT(sync_scroll(int)));

	connect(ui->leftdir->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->verticalScrollBar, SLOT(setValue(int)));
	connect(ui->rightdir->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->verticalScrollBar, SLOT(setValue(int)));
    connect(ui->leftdir, SIGNAL(itemSelectionChanged()), SLOT(copy_selection_to_right()));
    connect(ui->rightdir, SIGNAL(itemSelectionChanged()), SLOT(copy_selection_to_left()));

	ui->verticalScrollBar->setValue(0);

	connect(ui->leftdir, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(do_action(QListWidgetItem*)));
	connect(ui->rightdir, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(do_action(QListWidgetItem*)));
}

void CompareWidget::unsyncWindows()
{
	ui->leftdir->disconnect(this);
	ui->rightdir->disconnect(this);
	ui->leftdir->verticalScrollBar()->disconnect(ui->verticalScrollBar);
	ui->rightdir->verticalScrollBar()->disconnect(ui->verticalScrollBar);
	ui->verticalScrollBar->disconnect(this);
	ui->leftdir->verticalScrollBar()->disconnect(this);
}

bool CompareWidget::hidden(std::size_t i) const
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

void CompareWidget::applyFilters()
{
	// save the currently selected item
	const int   sel              = ui->leftdir->currentRow();
	std::size_t last_low_shown   = NOT_FOUND;
	std::size_t first_high_shown = NOT_FOUND;

	// for each item, check if it is shown or not, and adjust font
	for ( std::size_t i = 0, n = list.size(); i < n; ++i )
	{
		const bool hideitem = hidden(i);

		if ( QListWidgetItem * l = ui->leftdir->item(i))
		{
			style(l, hideitem, i);
		}

		if ( QListWidgetItem * r = ui->rightdir->item(i))
		{
			style(r, hideitem, i);
		}

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
		changesel(first_high_shown);
	}
	else if ( last_low_shown != NOT_FOUND )
	{
		changesel(last_low_shown);
	}
	else
	{
		clearSelection();
	}
}

void CompareWidget::items_compared(
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

void CompareWidget::style(
	QListWidgetItem* item,
	bool             hidden_,
	std::size_t      i
) const
{
	item->setHidden(hidden_);

	// strike out ignored items
	QFont f = item->font();
	f.setStrikeOut(list[i].ignore);
	f.setItalic(list[i].ignore);
	item->setFont(f);

	// set font colour
	QColor font_colour = Qt::gray;

	// green for unmatched items
	if ( list[i].unmatched())
	{
		font_colour = QColor(0x40, 0xA0, 0x40);
	}
	else
	{
		// matched items are gray for uncompared, black for the same, red for different
		if ( list[i].compared )
		{
			if ( list[i].same )
			{
				font_colour = Qt::black;
			}
			else
			{
				font_colour = QColor(0xD0, 0x40, 0x40);
			}
		}
	}

	item->setTextColor(font_colour);
}

void CompareWidget::clearFilter()
{
    setFilter(QRegExp());
}

void CompareWidget::setFilter(const QRegExp & r)
{
    filter = r;
    applyFilters();
}

void CompareWidget::clearSelection()
{
    ui->leftdir->setCurrentItem(0);
    ui->rightdir->setCurrentItem(0);
}

void copy_selection(QListWidget* from, QListWidget* to)
{
    const int m = from->count();

    for (int i = 0; i < m; ++i)
    {
        if (QListWidgetItem* item = from->item(i))
        {
            if (QListWidgetItem* jtem = to->item(i))
            {
                const bool sel = item->isSelected();

                if (jtem->isSelected() != sel)
                    jtem->setSelected(sel);
            }
        }
    }
}

void CompareWidget::copy_selection_to_left()
{
    ui->leftdir->disconnect(SIGNAL(itemSelectionChanged()),this, SLOT(copy_selection_to_right()));
    copy_selection(ui->rightdir, ui->leftdir);
    connect(ui->leftdir, SIGNAL(itemSelectionChanged()), SLOT(copy_selection_to_right()));
}

void CompareWidget::copy_selection_to_right()
{
    ui->rightdir->disconnect(SIGNAL(itemSelectionChanged()), this, SLOT(copy_selection_to_left()));
    copy_selection(ui->leftdir, ui->rightdir);
    connect(ui->rightdir, SIGNAL(itemSelectionChanged()), SLOT(copy_selection_to_left()));
}

void CompareWidget::changesel(int row)
{
    if ( ui->leftdir->currentRow() != row )
    {
        if ( QListWidgetItem * item = ui->leftdir->item(row))
        {
            ui->leftdir->setCurrentItem(item, QItemSelectionModel::ClearAndSelect);
            ui->leftdir->scrollToItem(item);
        }
    }

    if ( ui->rightdir->currentRow() != row )
    {
        if ( QListWidgetItem * item = ui->rightdir->item(row))
        {
            ui->rightdir->setCurrentItem(item, QItemSelectionModel::ClearAndSelect);
            ui->rightdir->scrollToItem(item);
        }
    }
}

void CompareWidget::sync_scroll(int val)
{
	if ( ui->leftdir->verticalScrollBar()->value() != val )
	{
		ui->leftdir->verticalScrollBar()->setValue(val);
	}

	if ( ui->rightdir->verticalScrollBar()->value() != val )
	{
		ui->rightdir->verticalScrollBar()->setValue(val);
	}
}

void CompareWidget::setScrollBarRange(
	int min_,
	int max_
)
{
	ui->verticalScrollBar->setRange(min_, max_);
}

QString CompareWidget::getCurrentLeft() const
{
	if ( QListWidgetItem * item = ui->leftdir->currentItem())
	{
		const int i = ui->leftdir->row(item);

		if ( i >= 0 )
		{
			return list[i].items.left;
		}
	}

	return QString();
}

QString CompareWidget::getCurrentRight() const
{
	if ( QListWidgetItem * item = ui->rightdir->currentItem())
	{
		const int i = ui->rightdir->row(item);

		if ( i >= 0 )
		{
			return list[i].items.right;
		}
	}

	return QString();
}

QStringList CompareWidget::getSelectedLeft() const
{
    QStringList l;

    QList< QListWidgetItem* > items = ui->leftdir->selectedItems();
    for (int j = 0; j < items.count(); ++j)
    {
        if (QListWidgetItem* item = items.at(j))
        {
            const int i = ui->leftdir->row(item);
            if ( i >= 0)
            {
                l << list[i].items.left;
            }
        }
    }

    return l;
}
QStringList CompareWidget::getSelectedRight() const
{
    QStringList l;

    QList< QListWidgetItem* > items = ui->rightdir->selectedItems();
    for (int j = 0; j < items.count(); ++j)
    {
        if (QListWidgetItem* item = items.at(j))
        {
            const int i = ui->rightdir->row(item);
            if ( i >= 0)
            {
                l << list[i].items.right;
            }
        }
    }

    return l;
}


void CompareWidget::do_action(QListWidgetItem* item)
{
	if ( item )
	{
		if ( QListWidget * w = item->listWidget())
		{
			const int x_ = w->row(item);

			if ( x_ >= 0 )
			{
				emit itemDoubleClicked(list[x_].items.left, list[x_].items.right);
			}
		}
	}
}

void CompareWidget::showOnlyLeft(bool checked)
{
	hide_left_only = !checked;
	applyFilters();
}

void CompareWidget::showIgnored(bool checked)
{
	hide_ignored = !checked;
	applyFilters();
}

void CompareWidget::showSame(bool checked)
{
	hide_identical_items = !checked;
	applyFilters();
}

void CompareWidget::showOnlyRight(bool checked)
{
	hide_right_only = !checked;
	applyFilters();
}

void CompareWidget::on_actionIgnore_triggered()
{
	int idx = ui->leftdir->currentRow();

	if ( idx < 0 )
	{
		idx = ui->rightdir->currentRow();
	}

	if ( idx >= 0 )
	{
		list[idx].ignore = true;
		applyFilters();
	}
}

void CompareWidget::on_actionCopy_To_Clipboard_triggered()
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

void CompareWidget::setLeftAndRight(
	const QString&     leftname_,
	const QString&     rightname_,
	const QStringList& leftitems,
	const QStringList& rightitems
)
{
	stopComparison();

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

	// Update display
	unsyncWindows();

	ui->leftdir->clear();
	ui->rightdir->clear();

	for ( std::size_t i = 0, n = list.size(); i < n; ++i )
	{
		const QString leftfile  = list[i].items.left;
		const QString rightfile = list[i].items.right;

		// create items
		if ( leftfile.isEmpty())
		{
			new QListWidgetItem(ui->leftdir);
		}
		else
		{
			new QListWidgetItem(leftfile, ui->leftdir);
		}

		if ( rightfile.isEmpty())
		{
			new QListWidgetItem(ui->rightdir);
		}
		else
		{
			new QListWidgetItem(rightfile, ui->rightdir);
		}
	}

	applyFilters();

	syncWindows();
}

std::vector< items_t > CompareWidget::match(
	const QStringList& leftitems,
	const QStringList& rightitems
) const
{
	std::map< int, std::vector< items_t > > score;

	for ( int il = 0, nl = leftitems.count(); il < nl; ++il )
	{
		for ( int ir = 0, nr = rightitems.count(); ir < nr; ++ir )
		{
			score[matcher->compare(leftitems.at(il), rightitems.at(ir))].push_back(
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

void CompareWidget::setComparison(const Compare& comp)
{
	if ( Compare * p = comp.clone())
	{
		stopComparison();
		delete compare;
		compare = p;

		for ( std::size_t i = 0; i < list.size(); ++i )
		{
			list[i].compared = false;
		}

		startComparison();

	}
}

void CompareWidget::startComparison()
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
		stopComparison();

		worker = new WorkerThread(compare, matches);
		connect(worker, SIGNAL(compared(QString, QString, bool)), SLOT(items_compared(QString, QString, bool)));
		worker->start();
	}
}

QStringList CompareWidget::getAllLeft() const
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

QStringList CompareWidget::getAllRight() const
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

void CompareWidget::updateLeft(
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
				QListWidgetItem* item = ui->leftdir->item(i);
				item->setText(QString());
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

			comparison_t y = { x };

			// insert into the list
			list.insert(list.begin() + j, y);

			ui->leftdir->insertItem(j, new QListWidgetItem(item));
			ui->rightdir->insertItem(j, new QListWidgetItem());
		}
	}

	// remove empty items
	for ( std::size_t i = 0; i < list.size();)
	{
		if ( list[i].items.right.isEmpty() && list[i].items.left.isEmpty())
		{
			QListWidgetItem* rightitem = ui->rightdir->takeItem(i);
			delete rightitem;
			QListWidgetItem* leftitem = ui->leftdir->takeItem(i);
			delete leftitem;
			list.erase(list.begin() + i);
		}
		else
		{
			++i;
		}
	}

	rematch();
}

void CompareWidget::updateRight(
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
				QListWidgetItem* item = ui->rightdir->item(i);
				item->setText(QString());
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

			comparison_t y = { x };

			// insert into the list
			list.insert(list.begin() + j, y);

			ui->rightdir->insertItem(j, new QListWidgetItem(item));
			ui->leftdir->insertItem(j, new QListWidgetItem());
		}
	}

	// remove empty items
	for ( std::size_t i = 0; i < list.size();)
	{
		if ( list[i].items.left.isEmpty() && list[i].items.right.isEmpty())
		{
			QListWidgetItem* leftitem = ui->leftdir->takeItem(i);
			delete leftitem;
			QListWidgetItem* rightitem = ui->rightdir->takeItem(i);
			delete rightitem;
			list.erase(list.begin() + i);
		}
		else
		{
			++i;
		}
	}

	rematch();
}

void CompareWidget::rematch()
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
			delete ui->leftdir->takeItem(i);
			delete ui->rightdir->takeItem(i);
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
			comparison_t c = { matching[i] };
			list.insert(list.begin() + i, c);

			ui->rightdir->insertItem(i, new QListWidgetItem(matching[i].right));
			ui->leftdir->insertItem(i, new QListWidgetItem(matching[i].left));
		}
	}

	startComparison();
	applyFilters();

}

// WorkerThread ================================================================
WorkerThread::WorkerThread(
    Compare*                      cmp,
    const std::vector< items_t >& t
)
	: QThread(0), compare(cmp ? cmp->clone() : 0), todo(t)
{
}

WorkerThread::~WorkerThread()
{
	clear();
	wait();
	delete compare;
}

void WorkerThread::clear()
{
	lock.lock();
	todo.clear();
	lock.unlock();
}

void WorkerThread::run()
{
	while ( true )
	{
		lock.lock();
		TaskList::iterator it = todo.begin();

		if ( it == todo.end())
		{
			lock.unlock();
			return;
		}
		else
		{
			QString left  = it->left;
			QString right = it->right;
			todo.erase(it);
			lock.unlock();

			const bool res = ( compare ? compare->equal(left, right) : false );
			emit       compared(left, right, res);
		}
	}
}
