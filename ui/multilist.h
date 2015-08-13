#ifndef MULTILIST_H
#define MULTILIST_H

#include <QWidget>
class QListWidgetItem;

namespace Ui {
class MultiList;
}

class MultiList : public QWidget
{
    Q_OBJECT

public:
    explicit MultiList(QWidget *parent = 0);
    ~MultiList();

    /** Change the currently selected item in both lists. Scroll to it
     */
    void changesel(int);

    /** Deselect all items
     */
    void clearSelection();

    QStringList currentText() const;

    void setItems(const QList< QPair< QString, QString > >&);

    void clearLeft(int);

    void clearRight(int);

    void remove(int);

    void insert(int, const QString&, const QString&);

    QStringList getSelectedLeft() const;
    QStringList getSelectedRight() const;

    void style(int, bool, bool, bool, bool, bool);

    int currentRow() const;
signals:
    void itemDoubleClicked(int);
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
private:
    /** Setup signals for keeping the two list widgets scrolled to the same point
     */
    void syncwindows();

    /** Stop the synchronization between the list widgets
     *
     * This function is necessary so that the GUI doesn't go wild as many
     * changes are made to the lists. It seems like a bit of a workaround.
     */
    void unsyncwindows();

    void styleitem(QListWidgetItem*, bool, bool, bool, bool, bool);

    Ui::MultiList *ui;
};

#endif // MULTILIST_H
