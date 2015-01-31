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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui
{
class MainWindow;
}

/** @brief The main window of the application
 *
 * Basically loads the widget and sets them to point at the appropriate
 * directories
 */
class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	/** @brief Initialize the comparison tool with the command line arguments
	 * @param directories One or two directories from the command line
     * @param show_left_only Whether to show files that are on the left only
     * @param show_right_only Whether to show files that are on the right only
     * @param show_identical Whether to show files that are the same on the left and righ
	 *
	 * If directories has no elements, both sides of the compare window will
	 * be the current directory. If it has one element, the left will be the
	 * current directory, and the right will be the given directory. If there
	 * are two or more elements, the left will be the first entry and the
	 * right will be the second.
	 */
    explicit MainWindow(const std::vector< std::string >& directories, bool show_left_only, bool show_right_only, bool show_identical);

	/** @brief Destructor
	 */
	~MainWindow();
private:
	Ui::MainWindow* ui;
};

#endif // MAINWINDOW_H
