#ifndef DIRECTORYCOMPARISON_H
#define DIRECTORYCOMPARISON_H

#include <QObject>
#include "compare.h"
#include "items.h"
#include "workerthread.h"
#include "directorycontents.h"
#include "matcher.h"

class DirectoryComparison : public QObject
{
    Q_OBJECT
public:
    explicit DirectoryComparison(QObject *parent = 0) : compare(0), worker(0), matcher(new DefaultMatcher)
    {

    }
    ~DirectoryComparison()
    {
        delete compare;
        delete matcher;
    }

    void setCompare(Compare* c)
    {
        delete compare;
        compare = c;
    }

    /** Interrupt the worker thread and stop doing any more comparisons
     */
    void stopComparison()
    {
        delete worker;
        worker = 0;
    }

    QString getLeftLocation() const
    {
        return ldir.absolutePath();
    }

    void setLeftLocation(const QString& s)
    {
        ldir.cd(s);
    }

    QString getLeftLocation(const QString& s) const
    {
        return ldir.absoluteFilePath(s);
    }

    QString getLeftName() const
    {
        return ldir.name();
    }

    QString getRightLocation() const
    {
        return rdir.absolutePath();
    }

    void setRightLocation(const QString& s)
    {
        rdir.cd(s);
    }

    QString getRightLocation(const QString& s) const
    {
        return rdir.absoluteFilePath(s);
    }

    QString getRightName() const
    {
        return rdir.name();
    }

    QPair<QStringList, QStringList> setDepth(int d)
    {
        return QPair< QStringList, QStringList >(ldir.setDepth(d), rdir.setDepth(d));
    }

    QStringList getLeftRelativeFileNames() const
    {
        return ldir.getRelativeFileNames();
    }

    QStringList getRightRelativeFileNames() const
    {
        return rdir.getRelativeFileNames();
    }

    QString getLeftRelativeFilePath(const QString& s) const
    {
        return ldir.relativeFilePath(s);
    }

    QString getRightRelativeFilePath(const QString& s) const
    {
        return rdir.relativeFilePath(s);
    }

    DirectoryContents::update_t updateLeft(const QString& s)
    {
        return ldir.update(s);
    }

    DirectoryContents::update_t updateRight(const QString& s)
    {
        return rdir.update(s);
    }

    QStringList getDirectories() const
    {
        QStringList l;
        l << ldir.getDirectories() << rdir.getDirectories();
        return l;
    }

    void startWorker(const std::vector< items_t >& matches)
    {
        worker = new WorkerThread(compare, matches);
        connect(worker, SIGNAL(compared(QString, QString, bool)), SIGNAL(compared(QString, QString, bool)));
        worker->start();
    }

    void setMatcher(const Matcher& m)
    {
        if ( Matcher * p = m.clone())
        {
            delete matcher;
            matcher = p;
        }

    }

    int match(const QString& l, const QString& r) const
    {
        return matcher->compare(l, r);
    }
signals:
    void compared(QString, QString, bool);

public slots:
private:
    /// Object that does the comparison of items
    Compare* compare;

    /// A thread that performs comparisons in the background
    WorkerThread* worker;

    /// Object that matches item names between left and right lists
    Matcher* matcher;

    DirectoryContents   ldir;
    DirectoryContents   rdir;
};

#endif // DIRECTORYCOMPARISON_H
