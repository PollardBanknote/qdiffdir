#include "multilist.h"
#include "ui_multilist.h"

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

/// @todo Display context menu from children
MultiList::MultiList(QWidget *parent) :
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
    ui->verticalScrollBar->setRange(ui->leftdir->verticalScrollBar()->minimum(), ui->leftdir->verticalScrollBar()->maximum());
    connect(ui->leftdir->verticalScrollBar(), SIGNAL(rangeChanged(int, int)), SLOT(setScrollBarRange(int, int)));
    connect(ui->verticalScrollBar, SIGNAL(valueChanged(int)), SLOT(sync_scroll(int)));

    connect(ui->leftdir->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->verticalScrollBar, SLOT(setValue(int)));
    connect(ui->rightdir->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->verticalScrollBar, SLOT(setValue(int)));
    connect(ui->leftdir, SIGNAL(itemSelectionChanged()), SLOT(copy_selection_to_right()));
    connect(ui->rightdir, SIGNAL(itemSelectionChanged()), SLOT(copy_selection_to_left()));

    ui->verticalScrollBar->setValue(0);

    connect(ui->leftdir, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(handle_item_double_clicked(QListWidgetItem*)));
    connect(ui->rightdir, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(handle_item_double_clicked(QListWidgetItem*)));
}

void MultiList::unsyncwindows()
{
    ui->leftdir->disconnect(this);
    ui->rightdir->disconnect(this);
    ui->leftdir->verticalScrollBar()->disconnect(ui->verticalScrollBar);
    ui->rightdir->verticalScrollBar()->disconnect(ui->verticalScrollBar);
    ui->verticalScrollBar->disconnect(this);
    ui->leftdir->verticalScrollBar()->disconnect(this);
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
    ui->leftdir->disconnect(SIGNAL(itemSelectionChanged()),this, SLOT(copy_selection_to_right()));
    copy_selection(ui->rightdir, ui->leftdir);
    connect(ui->leftdir, SIGNAL(itemSelectionChanged()), SLOT(copy_selection_to_right()));
}

void MultiList::copy_selection_to_right()
{
    ui->rightdir->disconnect(SIGNAL(itemSelectionChanged()), this, SLOT(copy_selection_to_left()));
    copy_selection(ui->leftdir, ui->rightdir);
    connect(ui->rightdir, SIGNAL(itemSelectionChanged()), SLOT(copy_selection_to_left()));
}

void MultiList::handle_item_double_clicked(QListWidgetItem * item)
{
    if (item)
    {
        if (QListWidget* w = item->listWidget())
        {
            emit itemDoubleClicked(w->row(item));
        }
    }
}

void MultiList::clearSelection()
{
    ui->leftdir->setCurrentItem(0);
    ui->rightdir->setCurrentItem(0);
}

void MultiList::changesel(int row)
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

QString get_text(QListWidget* w)
{
    if ( QListWidgetItem * item = w->currentItem())
    {
        return item->text();
    }

    return QString();
}

QStringList MultiList::currentText() const
{
    QStringList res;

    res << get_text(ui->leftdir) << get_text(ui->rightdir);

    return res;
}

void MultiList::setItems(const QList<QPair<QString, QString> > & items)
{

    // Update display
    unsyncwindows();

    ui->leftdir->clear();
    ui->rightdir->clear();

    for ( std::size_t i = 0, n = items.size(); i < n; ++i )
    {
        const QString leftfile  = items.at(i).first;
        const QString rightfile = items.at(i).second;

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

    syncwindows();

}

void MultiList::clearLeft(int i)
{
    if (QListWidgetItem* item = ui->leftdir->item(i))
        item->setText(QString());
}

void MultiList::clearRight(int i)
{
    if (QListWidgetItem* item = ui->rightdir->item(i))
        item->setText(QString());
}

void MultiList::remove(int i)
{
    if (QListWidgetItem* leftitem = ui->leftdir->takeItem(i))
        delete leftitem;
    if (QListWidgetItem* rightitem = ui->rightdir->takeItem(i))
        delete rightitem;
}

void MultiList::insert(int j, const QString & l, const QString & r)
{
    ui->leftdir->insertItem(j, new QListWidgetItem(l));
    ui->rightdir->insertItem(j, new QListWidgetItem(r));
}

QStringList MultiList::getSelectedLeft() const
{
    QStringList l;

    QList< QListWidgetItem* > items = ui->leftdir->selectedItems();
    for (int j = 0; j < items.count(); ++j)
    {
        if (QListWidgetItem* item = items.at(j))
        {
            l << item->text();
        }
    }

    return l;
}

QStringList MultiList::getSelectedRight() const
{
    QStringList l;

    QList< QListWidgetItem* > items = ui->rightdir->selectedItems();
    for (int j = 0; j < items.count(); ++j)
    {
        if (QListWidgetItem* item = items.at(j))
        {
            l << item->text();
        }
    }

    return l;
}

void MultiList::style(int i, bool hidden, bool ignored, bool unmatched, bool compared, bool same)
{
    if ( QListWidgetItem * l = ui->leftdir->item(i))
    {
        styleitem(l, hidden, ignored, unmatched, compared, same);
    }

    if ( QListWidgetItem * r = ui->rightdir->item(i))
    {
        styleitem(r, hidden, ignored, unmatched, compared, same);
    }
}

void MultiList::styleitem(
    QListWidgetItem* item,
    bool             hidden_,
        bool ignore_,
        bool unmatched_,
        bool compared_,
        bool same_
)
{
    item->setHidden(hidden_);

    // strike out ignored items
    QFont f = item->font();
    f.setStrikeOut(ignore_);
    f.setItalic(ignore_);
    item->setFont(f);

    // set font colour
    QColor font_colour = Qt::gray;

    // green for unmatched items
    if ( unmatched_)
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

