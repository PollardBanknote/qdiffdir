#include "workerthread.h"

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
