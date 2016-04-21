/* Copyright (c) 2015, Pollard Banknote Limited
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
#include "multilist.h"

#include <QHBoxLayout>


void copy_selection(
	QListWidget* from,
	QListWidget* to
)
{
	const int m = from->count();

	for ( int i = 0; i < m; ++i )
	{
		if ( QListWidgetItem * item = from->item(i))
		{
			if ( QListWidgetItem * jtem = to->item(i))
			{
				const bool sel = item->isSelected();

				if ( jtem->isSelected() != sel )
				{
					jtem->setSelected(sel);
				}
			}
		}
	}
}

/// @todo Display context menu from children
MultiList::MultiList(QWidget* parent) :
	QWidget(parent)
{
	QHBoxLayout* layout = new QHBoxLayout;

	setLayout(layout);

	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);

	leftdir = new QListWidget(this);
	leftdir->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	leftdir->setSelectionMode(QAbstractItemView::ExtendedSelection);
	layout->addWidget(leftdir);

	verticalScrollBar = new QScrollBar(Qt::Vertical, this);
	layout->addWidget(verticalScrollBar);

	rightdir = new QListWidget(this);
	rightdir->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	rightdir->setSelectionMode(QAbstractItemView::ExtendedSelection);
	layout->addWidget(rightdir);

	syncwindows();
}

MultiList::~MultiList()
{
}

void MultiList::syncwindows()
{
	// Keep scrollbar in sync
	verticalScrollBar->setRange(leftdir->verticalScrollBar()->minimum(), leftdir->verticalScrollBar()->maximum());
	connect(leftdir->verticalScrollBar(), SIGNAL(rangeChanged(int, int)), SLOT(setScrollBarRange(int, int)));
	connect(verticalScrollBar, SIGNAL(valueChanged(int)), SLOT(sync_scroll(int)));
	connect(leftdir->verticalScrollBar(), SIGNAL(valueChanged(int)), verticalScrollBar, SLOT(setValue(int)));
	connect(rightdir->verticalScrollBar(), SIGNAL(valueChanged(int)), verticalScrollBar, SLOT(setValue(int)));
	verticalScrollBar->setValue(0);

	// Keep selections in sync
	connect(leftdir, SIGNAL(itemSelectionChanged()), SLOT(copy_selection_to_right()));
	connect(rightdir, SIGNAL(itemSelectionChanged()), SLOT(copy_selection_to_left()));
	connect(leftdir, SIGNAL(currentRowChanged(int)), SLOT(left_current_row_changed(int)));
	connect(rightdir, SIGNAL(currentRowChanged(int)), SLOT(right_current_row_changed(int)));

	connect(leftdir, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(handle_item_double_clicked(QListWidgetItem*)));
	connect(rightdir, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(handle_item_double_clicked(QListWidgetItem*)));
}

void MultiList::setScrollBarRange(
	int min_,
	int max_
)
{
	verticalScrollBar->setRange(min_, max_);
}

void MultiList::sync_scroll(int val)
{
	if ( leftdir->verticalScrollBar()->value() != val )
	{
		leftdir->verticalScrollBar()->setValue(val);
	}

	if ( rightdir->verticalScrollBar()->value() != val )
	{
		rightdir->verticalScrollBar()->setValue(val);
	}
}

void MultiList::copy_selection_to_left()
{
	leftdir->disconnect(SIGNAL(itemSelectionChanged()), this, SLOT(copy_selection_to_right()));
	copy_selection(rightdir, leftdir);
	connect(leftdir, SIGNAL(itemSelectionChanged()), SLOT(copy_selection_to_right()));
}

void MultiList::copy_selection_to_right()
{
	rightdir->disconnect(SIGNAL(itemSelectionChanged()), this, SLOT(copy_selection_to_left()));
	copy_selection(leftdir, rightdir);
	connect(rightdir, SIGNAL(itemSelectionChanged()), SLOT(copy_selection_to_left()));
}

void MultiList::handle_item_double_clicked(QListWidgetItem* item)
{
	if ( item )
	{
		if ( QListWidget * w = item->listWidget())
		{
			emit itemActivated(w->row(item));
		}
	}
}

void MultiList::clearSelection()
{
	leftdir->setCurrentItem(0);
	rightdir->setCurrentItem(0);
}

QString get_text(QListWidget* w)
{
	if ( QListWidgetItem * item = w->currentItem())
	{
		return item->text();
	}

	return QString();
}

void MultiList::clear()
{
	leftdir->clear();
	rightdir->clear();
}

void MultiList::addItem(
	const QString& leftfile,
	const QString& rightfile
)
{
	// create items
	if ( leftfile.isEmpty())
	{
		new QListWidgetItem(leftdir);
	}
	else
	{
		new QListWidgetItem(leftfile, leftdir);
	}

	if ( rightfile.isEmpty())
	{
		new QListWidgetItem(rightdir);
	}
	else
	{
		new QListWidgetItem(rightfile, rightdir);
	}
}

void MultiList::clearLeft(int i)
{
	if ( QListWidgetItem * item = leftdir->item(i))
	{
		item->setText(QString());
	}
}

void MultiList::clearRight(int i)
{
	if ( QListWidgetItem * item = rightdir->item(i))
	{
		item->setText(QString());
	}
}

void MultiList::removeItem(int i)
{
	if ( QListWidgetItem * leftitem = leftdir->takeItem(i))
	{
		delete leftitem;
	}

	if ( QListWidgetItem * rightitem = rightdir->takeItem(i))
	{
		delete rightitem;
	}
}

void MultiList::insertItem(
	int            j,
	const QString& l,
	const QString& r
)
{
	leftdir->insertItem(j, new QListWidgetItem(l));
	rightdir->insertItem(j, new QListWidgetItem(r));
}

void MultiList::style(
	int  i,
	bool ignored,
	bool unmatched,
	bool compared,
	bool same
)
{
	if ( QListWidgetItem * l = leftdir->item(i))
	{
		styleitem(l, ignored, unmatched, compared, same);
	}

	if ( QListWidgetItem * r = rightdir->item(i))
	{
		styleitem(r, ignored, unmatched, compared, same);
	}
}

void MultiList::setRowHidden(
	int  i,
	bool hidden
)
{
	if ( QListWidgetItem * l = leftdir->item(i))
	{
		l->setHidden(hidden);
	}

	if ( QListWidgetItem * r = rightdir->item(i))
	{
		r->setHidden(hidden);
	}
}

void MultiList::styleitem(
	QListWidgetItem* item,
	bool             ignore_,
	bool             unmatched_,
	bool             compared_,
	bool             same_
)
{
	// strike out ignored items
	QFont f = item->font();

	f.setStrikeOut(ignore_);
	f.setItalic(ignore_);
	item->setFont(f);

	// set font colour
	QColor font_colour = Qt::gray;

	// green for unmatched items
	if ( unmatched_ )
	{
		font_colour = QColor(0x40, 0xA0, 0x40);
	}
	else
	{
		// matched items are gray for uncompared, black for the same, red for different
		if ( compared_ )
		{
			if ( same_ )
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

int MultiList::currentRow() const
{
	int idx = leftdir->currentRow();

	if ( idx < 0 )
	{
		idx = rightdir->currentRow();
	}

	return idx;
}

QList< int > MultiList::selectedRows() const
{
	QList< int > l;

	QList< QListWidgetItem* > items = rightdir->selectedItems();

	for ( int i = 0, n = items.count(); i < n; ++i )
	{
		l << rightdir->row(items.at(i));
	}

	return l;
}

void MultiList::setSelectedRows(const QList< int >& l)
{
	if ( l.isEmpty())
	{
		clearSelection();
	}
	else
	{
		// temporarily disonnect signals that edit selection
		leftdir->disconnect(SIGNAL(itemSelectionChanged()), this, SLOT(copy_selection_to_right()));
		rightdir->disconnect(SIGNAL(itemSelectionChanged()), this, SLOT(copy_selection_to_left()));

		leftdir->setCurrentRow(l.at(0));
		rightdir->setCurrentRow(l.at(0));

		for ( int i = 0, n = leftdir->count(); i < n; ++i )
		{
			const bool sel = l.contains(i);

			if ( QListWidgetItem * item = leftdir->item(i))
			{
				item->setSelected(sel);
			}

			if ( QListWidgetItem * item = rightdir->item(i))
			{
				item->setSelected(sel);
			}
		}

		connect(leftdir, SIGNAL(itemSelectionChanged()), SLOT(copy_selection_to_right()));
		connect(rightdir, SIGNAL(itemSelectionChanged()), SLOT(copy_selection_to_left()));
	}
}

void MultiList::left_current_row_changed(int idx)
{
	rightdir->setCurrentRow(idx, QItemSelectionModel::NoUpdate);
}

void MultiList::right_current_row_changed(int idx)
{
	leftdir->setCurrentRow(idx, QItemSelectionModel::NoUpdate);
}
