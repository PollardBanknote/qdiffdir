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
#ifndef DIRECTORYCOMPARISON_H
#define DIRECTORYCOMPARISON_H

#include <vector>

#include <QObject>
#include <QPair>

#include "items.h"
#include "trees/directorycontents.h"
class Matcher;
class WorkerThread;
class Compare;

class DirectoryComparison
	: public QObject
{
	Q_OBJECT
public:
	explicit DirectoryComparison(QObject* parent = 0);

	~DirectoryComparison();

	void setCompare(Compare* c);

	/** Interrupt the worker thread and stop doing any more comparisons
	 */
	void stopComparison();

    std::string getLeftLocation() const;

    void setLeftLocation(const std::string& s);

    std::string getLeftLocation(const std::string& s) const;

    std::string getLeftName() const;

    std::string getRightLocation() const;

    void setRightLocation(const std::string& s);

    std::string getRightLocation(const std::string& s) const;

    std::string getRightName() const;

    std::pair< std::vector< std::string >, std::vector< std::string > > setDepth(int d);

    std::vector< std::string > getLeftRelativeFileNames() const;

    std::vector< std::string > getRightRelativeFileNames() const;

    std::string getLeftRelativeFilePath(const std::string& s) const;

    std::string getRightRelativeFilePath(const std::string& s) const;

    DirectoryContents::update_t updateLeft(const std::string &s);

    DirectoryContents::update_t updateRight(const std::string &s);

    std::vector<std::string> getDirectories() const;

	void startWorker(const std::vector< items_t >& matches);

	void setMatcher(const Matcher& m);

    int match(const std::string& l, const std::string& r) const;
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

	DirectoryContents ldir;
	DirectoryContents rdir;
};

#endif // DIRECTORYCOMPARISON_H
