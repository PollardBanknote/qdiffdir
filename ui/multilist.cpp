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
#include "ui_multilist.h"

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
	QWidget(parent),
	ui(new Ui::MultiList)
{
	ui->setupUi(this);

	syncwindows();
}

MultiList::~MultiList()
{
	delete ui;
}

void MultiList::syncwindows()
{
    // Keep scrollbar in sync
	ui->verticalScrollBar->setRange(ui->leftdir->verticalScrollBar()->minimum(), ui->leftdir->verticalScrollBar()->maximum());
	connect(ui->leftdir->verticalScrollBar(), SIGNAL(rangeChanged(int, int)), SLOT(setScrollBarRange(int, int)));
	connect(ui->verticalScrollBar, SIGNAL(valueChanged(int)), SLOT(sync_scroll(int)));
	connect(ui->leftdir->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->verticalScrollBar, SLOT(setValue(int)));
	connect(ui->rightdir->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->verticalScrollBar, SLOT(setValue(int)));
    ui->verticalScrollBar->setValue(0);

    // Keep selections in sync
	connect(ui->leftdir, SIGNAL(itemSelectionChanged()), SLOT(copy_selection_to_right()));
	connect(ui->rightdir, SIGNAL(itemSelectionChanged()), SLOT(copy_selection_to_left()));
    connect(ui->leftdir,SIGNAL(currentRowChanged(int)), SLOT(left_current_row_changed()));
    connect(ui->rightdir, SIGNAL(currentRowChanged(int)), SLOT(right_current_row_changed()));

    connect(ui->leftdir, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(handle_item_double_clicked(QListWidgetItem*)));
    connect(ui->rightdir, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(handle_item_double_clicked(QListWidgetItem*)));
}

void MultiList::setScrollBarRange(
	int min_,
	int max_
)
{
	ui->verticalScrollBar->setRange(min_, max_);
}

void MultiList::sync_scroll(int val)
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

void MultiList::copy_selection_to_left()
{
	ui->leftdir->disconnect(SIGNAL(itemSelectionChanged()), this, SLOT(copy_selection_to_right()));
	copy_selection(ui->rightdir, ui->leftdir);
	connect(ui->leftdir, SIGNAL(itemSelectionChanged()), SLOT(copy_selection_to_right()));
}

void MultiList::copy_selection_to_right()
{
	ui->rightdir->disconnect(SIGNAL(itemSelectionChanged()), this, SLOT(copy_selection_to_left()));
	copy_selection(ui->leftdir, ui->rightdir);
	connect(ui->rightdir, SIGNAL(itemSelectionChanged()), SLOT(copy_selection_to_left()));
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
	ui->leftdir->setCurrentItem(0);
	ui->rightdir->setCurrentItem(0);
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
    ui->leftdir->clear();
    ui->rightdir->clear();
}

void MultiList::addItem(const QString &leftfile, const QString &rightfile)
{
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

void MultiList::clearLeft(int i)
{
	if ( QListWidgetItem* item = ui->leftdir->item(i))
	{
		item->setText(QString());
	}
}

void MultiList::clearRight(int i)
{
	if ( QListWidgetItem* item = ui->rightdir->item(i))
	{
		item->setText(QString());
	}
}

void MultiList::removeItem(int i)
{
	if ( QListWidgetItem* leftitem = ui->leftdir->takeItem(i))
	{
		delete leftitem;
	}

	if ( QListWidgetItem* rightitem = ui->rightdir->takeItem(i))
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
	ui->leftdir->insertItem(j, new QListWidgetItem(l));
	ui->rightdir->insertItem(j, new QListWidgetItem(r));
}

void MultiList::style(
	int  i,
	bool ignored,
	bool unmatched,
	bool compared,
	bool same
)
{
	if ( QListWidgetItem* l = ui->leftdir->item(i))
	{
        styleitem(l, ignored, unmatched, compared, same);
	}

	if ( QListWidgetItem* r = ui->rightdir->item(i))
	{
        styleitem(r, ignored, unmatched, compared, same);
	}
}

void MultiList::setRowHidden(int i, bool hidden)
{
    if ( QListWidgetItem* l = ui->leftdir->item(i))
    {
        l->setHidden(hidden);
    }

    if ( QListWidgetItem* r = ui->rightdir->item(i))
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
    int idx = ui->leftdir->currentRow();

    if ( idx < 0 )
    {
        idx = ui->rightdir->currentRow();
    }

    return idx;
}

QList< int > MultiList::selectedRows() const
{
	QList< int > l;

	QList< QListWidgetItem* > items = ui->rightdir->selectedItems();
	for (int i = 0, n = items.count(); i < n; ++i)
	{
		l << ui->rightdir->row(items.at(i));
	}
	
	return l;
}

void MultiList::setSelectedRows(const QList<int> & l)
{
	if (l.isEmpty())
		clearSelection();
	else
	{
		// temporarily disonnect signals that edit selection
		ui->leftdir->disconnect(SIGNAL(itemSelectionChanged()), this, SLOT(copy_selection_to_right()));
		ui->rightdir->disconnect(SIGNAL(itemSelectionChanged()), this, SLOT(copy_selection_to_left()));
		
		ui->leftdir->setCurrentRow(l.at(0));
		ui->rightdir->setCurrentRow(l.at(0));
		
		for (int i = 0, n = ui->leftdir->count(); i < n; ++i)
		{
			const bool sel = l.contains(i);

			if (QListWidgetItem* item = ui->leftdir->item(i))
			{
				item->setSelected(sel);
			}
			if (QListWidgetItem* item = ui->rightdir->item(i))
			{
				item->setSelected(sel);
			}
		}
		
		connect(ui->leftdir, SIGNAL(itemSelectionChanged()), SLOT(copy_selection_to_right()));
		connect(ui->rightdir, SIGNAL(itemSelectionChanged()), SLOT(copy_selection_to_left()));
	}
}

void MultiList::left_current_row_changed()
{
    ui->rightdir->setCurrentRow(-1);
}

void MultiList::right_current_row_changed()
{
    ui->leftdir->setCurrentRow(-1);
}

