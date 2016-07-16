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

#include <iostream>

#include <QHBoxLayout>
#include <QListWidget>
#include <QScrollBar>


void copy_selection(
	QListWidget* from,
	QListWidget* to
)
{
	const int m = from->count();

	for ( int i = 0; i < m; ++i )
	{
		if ( QListWidgetItem* item = from->item(i))
		{
			if ( QListWidgetItem* jtem = to->item(i))
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
	setContextMenuPolicy(Qt::ActionsContextMenu);
	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);

	setLayout(layout);

	for ( int i = 0; i < 2; ++i )
	{
		if ( QListWidget* dir = new QListWidget(this))
		{
			dirs.push_back(dir);
			dir->setContextMenuPolicy(Qt::NoContextMenu);
			dir->setSelectionMode(QAbstractItemView::ExtendedSelection);

			// Sync scrollbar
            if (i == 0)
            {
                connect(dir->verticalScrollBar(), SIGNAL(valueChanged(int)), SLOT(sync_scroll(int)));
            }
            else
            {
                dir->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            }

			// Keep selections in sync
			connect(dir, SIGNAL(itemSelectionChanged()), SLOT(update_selection()));
			connect(dir, SIGNAL(currentRowChanged(int)), SLOT(current_row_changed(int)));
			connect(dir, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(handle_item_double_clicked(QListWidgetItem*)));

			this->layout()->addWidget(dir);
		}
	}

	update_scroll_range();
}

MultiList::~MultiList()
{
}

void MultiList::sync_scroll(int val)
{
    for ( std::size_t i = 1; i < dirs.size(); ++i )
	{
		if ( QListWidget* dir = dirs[i] )
		{
            dir->verticalScrollBar()->setValue(val);
		}
	}
}

void MultiList::update_selection()
{
	if ( QListWidget* from = dynamic_cast< QListWidget* >( sender()))
	{
		for ( std::size_t i = 0; i < dirs.size(); ++i )
		{
			if ( dirs[i] != from )
			{
				dirs[i]->disconnect(SIGNAL(itemSelectionChanged()), this, SLOT(update_selection()));
				copy_selection(from, dirs[i]);
				connect(dirs[i], SIGNAL(itemSelectionChanged()), SLOT(update_selection()));
			}
		}
	}
}

void MultiList::handle_item_double_clicked(QListWidgetItem* item)
{
	if ( item )
	{
		if ( QListWidget* w = item->listWidget())
		{
			emit itemActivated(w->row(item));
		}
	}
}

void MultiList::clearSelection()
{
	for ( std::size_t i = 0; i < dirs.size(); ++i )
	{
		dirs[i]->setCurrentItem(0);
	}
}

QString get_text(QListWidget* w)
{
	if ( QListWidgetItem* item = w->currentItem())
	{
		return item->text();
	}

	return QString();
}

void MultiList::clear()
{
	for ( std::size_t i = 0; i < dirs.size(); ++i )
	{
		dirs[i]->clear();
	}
    update_scroll_range();
}

void MultiList::addItem(const QStringList& items)
{
	const std::size_t n = items.count();

	for ( std::size_t i = 0; i < dirs.size(); ++i )
	{
		if ( i < n && !items.at(i).isEmpty())
		{
			new QListWidgetItem(items.at(i), dirs[i]);
		}
		else
		{
			new QListWidgetItem(dirs[i]);
		}
	}

	update_scroll_range();
}

void MultiList::update_scroll_range()
{
    /*
	if ( !dirs.empty())
	{
		if ( QScrollBar* sb = dirs[0]->verticalScrollBar())
		{
            std::cout << sb->minimum() << " : " << sb->maximum() << std::endl;
			verticalScrollBar->setRange(sb->minimum(), sb->maximum());
		}
	}
    */
}

void MultiList::clearText(
	int col,
	int row
)
{
	if ( col >= 0 && static_cast< unsigned >( col ) < dirs.size())
	{
		if ( QListWidgetItem* item = dirs[col]->item(row))
		{
			item->setText(QString());
		}
	}
}

void MultiList::removeItem(int r)
{
	for ( std::size_t i = 0; i < dirs.size(); ++i )
	{
		if ( QListWidgetItem* item = dirs[i]->takeItem(r))
		{
			delete item;
		}
	}

	update_scroll_range();
}

void MultiList::insertItem(
	int                j,
	const QStringList& l
)
{
	for ( std::size_t i = 0, n = dirs.size(), m = l.count(); i < n && i < m; ++i )
	{
		dirs[i]->insertItem(j, new QListWidgetItem(l.at(i)));
	}

	update_scroll_range();
}

void MultiList::style(
	int  r,
	bool ignored,
	bool unmatched,
	bool compared,
	bool same
)
{
	for ( std::size_t i = 0; i < dirs.size(); ++i )
	{
		if ( QListWidgetItem* item = dirs[i]->item(r))
		{
			styleitem(item, ignored, unmatched, compared, same);
		}
	}
}

void MultiList::setRowHidden(
	int  r,
	bool hidden
)
{
	for ( std::size_t i = 0; i < dirs.size(); ++i )
	{
		if ( QListWidgetItem* item = dirs[i]->item(r))
		{
			item->setHidden(hidden);
		}
	}

	update_scroll_range();
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
	for ( std::size_t i = 0; i < dirs.size(); ++i )
	{
		const int idx = dirs[i]->currentRow();

		if ( idx >= 0 )
		{
			return idx;
		}
	}

	return -1;
}

QList< int > MultiList::selectedRows() const
{
	QList< int > l;

	if ( !dirs.empty())
	{
		QList< QListWidgetItem* > items = dirs[0]->selectedItems();

		for ( int i = 0, n = items.count(); i < n; ++i )
		{
			l << dirs[0]->row(items.at(i));
		}
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
		for ( std::size_t r = 0; r < dirs.size(); ++r )
		{
			dirs[r]->disconnect(SIGNAL(itemSelectionChanged()), this, SLOT(update_selection()));
		}

		for ( std::size_t r = 0; r < dirs.size(); ++r )
		{
			dirs[r]->setCurrentRow(l.at(0));

			for ( int i = 0, n = dirs[r]->count(); i < n; ++i )
			{
				const bool sel = l.contains(i);

				if ( QListWidgetItem* item = dirs[r]->item(i))
				{
					item->setSelected(sel);
				}
			}
		}

		// Reconnect slots
		for ( std::size_t r = 0; r < dirs.size(); ++r )
		{
			connect(dirs[r], SIGNAL(itemSelectionChanged()), SLOT(update_selection()));
		}
	}
}

void MultiList::current_row_changed(int idx)
{
	if ( QObject* from = sender())
	{
		for ( std::size_t i = 0; i < dirs.size(); ++i )
		{
			if ( dirs[i] != from )
			{
				dirs[i]->setCurrentRow(idx, QItemSelectionModel::NoUpdate);
			}
		}
	}
}
