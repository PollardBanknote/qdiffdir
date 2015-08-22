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
#ifndef MULTILIST_H
#define MULTILIST_H

#include <QWidget>
class QListWidgetItem;

namespace Ui
{
class MultiList;
}

class MultiList : public QWidget
{
	Q_OBJECT
public:
	explicit MultiList(QWidget* parent = 0);
	~MultiList();

    void addItem(const QString& left, const QString& right);

    void insertItem(int, const QString&, const QString&);

    void removeItem(int);

    void clear();

	void clearLeft(int);

	void clearRight(int);

    void style(int, bool, bool, bool, bool);

    int currentRow() const;

    QList< int > selectedRows() const;

    void setSelectedRows(const QList<int>&);

    /** Deselect all items
     */
    void clearSelection();

    void setRowHidden(int, bool);
signals:
    void itemActivated(int);
private slots:
	void handle_item_double_clicked(QListWidgetItem*);

	/** Change the scroll bar range to match the list widgets
	 */
	void setScrollBarRange(int, int);

	/** Scroll the left and right lists to the same point
	 */
	void sync_scroll(int);

	void copy_selection_to_left();
	void copy_selection_to_right();
    void left_current_row_changed();
    void right_current_row_changed();
private:
	/** Setup signals for keeping the two list widgets scrolled to the same point
	 */
	void syncwindows();

    void styleitem(QListWidgetItem*, bool, bool, bool, bool);

	Ui::MultiList* ui;
};

#endif // MULTILIST_H
