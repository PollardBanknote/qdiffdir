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

    ui->multilistview->addAction(ui->actionIgnore);
    ui->multilistview->addAction(ui->actionCopy_To_Clipboard);
    connect(ui->multilistview, SIGNAL(itemDoubleClicked(int)), SLOT(do_action(int)));
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

void CompareWidget::clearFilter()
{
    setFilter(QRegExp());
}

void CompareWidget::setFilter(const QRegExp & r)
{
    filter = r;
    applyFilters();
}

QStringList CompareWidget::getSelectedLeft() const
{
    return ui->multilistview->getSelectedLeft();
}
QStringList CompareWidget::getSelectedRight() const
{
    return ui->multilistview->getSelectedRight();
}


void CompareWidget::do_action(int x_)
{
    if ( x_ >= 0 )
    {
        emit itemDoubleClicked(list[x_].items.left, list[x_].items.right);
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
    int idx = ui->multilistview->currentRow();

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

    QList< QPair< QString, QString > > items;

    for ( std::size_t i = 0, n = list.size(); i < n; ++i )
    {
        items.append(qMakePair(list[i].items.left, list[i].items.right));
    }

    ui->multilistview->setItems(items);

    applyFilters();
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

			comparison_t y = { x };

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

			comparison_t y = { x };

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
			comparison_t c = { matching[i] };
			list.insert(list.begin() + i, c);

            ui->multilistview->insert(i, matching[i].left, matching[i].right);
		}
	}

	startComparison();
	applyFilters();

}

QStringList CompareWidget::currentText() const
{
    return ui->multilistview->currentText();
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


