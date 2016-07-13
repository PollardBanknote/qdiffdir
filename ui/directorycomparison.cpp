/* Copyright (c) 2014, Pollard Banknote Limited
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
#include "directorycomparison.h"
#include "compare.h"
#include "matcher.h"
#include "workerthread.h"


DirectoryComparison::DirectoryComparison(QObject* parent)
    : QObject(parent), compare(0), worker(0), matcher(new DefaultMatcher)
{

}

DirectoryComparison::~DirectoryComparison()
{
	delete compare;
	delete matcher;
}

int DirectoryComparison::match(
	const QString& l,
	const QString& r
) const
{
	return matcher->compare(l, r);
}

void DirectoryComparison::setMatcher(const Matcher& m)
{
	if ( Matcher* p = m.clone())
	{
		delete matcher;
		matcher = p;
	}
}

void DirectoryComparison::startWorker(const std::vector< items_t >& matches)
{
	worker = new WorkerThread(compare, matches);
	connect(worker, SIGNAL(compared(QString, QString, bool)), SIGNAL(compared(QString, QString, bool)));
	worker->start();
}

QStringList DirectoryComparison::getDirectories() const
{
	QStringList l;

	l << ldir.getDirectories() << rdir.getDirectories();
	return l;
}

DirectoryContents::update_t DirectoryComparison::updateRight(const QString& s)
{
	return rdir.update(s);
}

DirectoryContents::update_t DirectoryComparison::updateLeft(const QString& s)
{
	return ldir.update(s);
}

QString DirectoryComparison::getRightRelativeFilePath(const QString& s) const
{
	return rdir.relativeFilePath(s);
}

QString DirectoryComparison::getLeftRelativeFilePath(const QString& s) const
{
	return ldir.relativeFilePath(s);
}

QStringList DirectoryComparison::getRightRelativeFileNames() const
{
	return rdir.getRelativeFileNames();
}

QStringList DirectoryComparison::getLeftRelativeFileNames() const
{
	return ldir.getRelativeFileNames();
}

QPair< QStringList, QStringList > DirectoryComparison::setDepth(int d)
{
	return QPair< QStringList, QStringList >(ldir.setDepth(d), rdir.setDepth(d));
}

QString DirectoryComparison::getRightName() const
{
	return rdir.name();
}

QString DirectoryComparison::getRightLocation(const QString& s) const
{
	return rdir.absoluteFilePath(s);
}

void DirectoryComparison::setRightLocation(const QString& s)
{
	rdir.cd(s);
}

QString DirectoryComparison::getRightLocation() const
{
	return rdir.absolutePath();
}

QString DirectoryComparison::getLeftName() const
{
	return ldir.name();
}

QString DirectoryComparison::getLeftLocation(const QString& s) const
{
	return ldir.absoluteFilePath(s);
}

void DirectoryComparison::setLeftLocation(const QString& s)
{
	ldir.cd(s);
}

QString DirectoryComparison::getLeftLocation() const
{
	return ldir.absolutePath();
}

void DirectoryComparison::stopComparison()
{
	delete worker;
	worker = 0;
}

void DirectoryComparison::setCompare(Compare* c)
{
	delete compare;
	compare = c;
}
