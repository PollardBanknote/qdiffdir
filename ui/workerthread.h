#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QThread>
#include <QMutex>
#include "compare.h"
#include "items.h"

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

#endif // WORKERTHREAD_H
