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
