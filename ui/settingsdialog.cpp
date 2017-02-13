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
#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QInputDialog>
#include <QMessageBox>

#include "mysettings.h"

SettingsDialog::SettingsDialog(QWidget* parent) :
	QDialog(parent),
	ui(new Ui::SettingsDialog)
{
	ui->setupUi(this);

	MySettings& settings = MySettings::instance();
	ui->diffToolLineEdit->setText(settings.getDiffTool());
	ui->editorLineEdit->setText(settings.getEditor());

	const QMap< QString, QString > filters = settings.getFilters();
	int                            nrows   = 0;

	for ( QMap< QString, QString >::const_iterator it = filters.constBegin(); it != filters.constEnd(); ++it )
	{
		ui->filter_list->insertRow(nrows);
		QTableWidgetItem* first  = new QTableWidgetItem(it.key());
		QTableWidgetItem* second = new QTableWidgetItem(it.value());
		ui->filter_list->setItem(nrows, 0, first);
		ui->filter_list->setItem(nrows, 1, second);
		++nrows;
	}
}

SettingsDialog::~SettingsDialog()
{
	delete ui;
}

void SettingsDialog::on_save_clicked()
{
	MySettings& settings = MySettings::instance();

	settings.setDiffTool(ui->diffToolLineEdit->text());
	settings.setEditor(ui->editorLineEdit->text());

	QMap< QString, QString > m;

	for ( int i = 0, r = ui->filter_list->rowCount(); i < r; ++i )
	{
		m.insert(ui->filter_list->item(i, 0)->text(), ui->filter_list->item(i, 1)->text());
	}

	settings.setFilters(m);

	accept();
}

void SettingsDialog::on_cancel_clicked()
{
	reject();
}

void SettingsDialog::accept()
{
	QDialog::accept();
}

void SettingsDialog::on_add_filter_clicked()
{
	const QString key = QInputDialog::getText(this, "Filter Name", "Please enter a name for the filter");

	if ( !key.isEmpty())
	{
		const QString value = QInputDialog::getText(this, "Filter", "Please enter a filter (ex., \"*.cpp; *.h\")");

		if ( !value.isEmpty())
		{
			const int r = ui->filter_list->rowCount();
			ui->filter_list->insertRow(r);

			QTableWidgetItem* first  = new QTableWidgetItem(key);
			QTableWidgetItem* second = new QTableWidgetItem(value);
			ui->filter_list->setItem(r, 0, first);
			ui->filter_list->setItem(r, 1, second);
		}
	}
}

void SettingsDialog::on_remove_filter_clicked()
{
	QList< QTableWidgetItem* > items = ui->filter_list->selectedItems();

	if ( !items.isEmpty())
	{
		if ( QMessageBox::question(this, "Delete filter", "Are you sure?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes )
		{
			ui->filter_list->removeRow(items.at(0)->row());
		}
	}
}

void SettingsDialog::on_edit_filter_clicked()
{
	QList< QTableWidgetItem* > items = ui->filter_list->selectedItems();

	for ( int i = 0; i < items.count(); ++i )
	{
		if ( QTableWidgetItem* item = items.at(i))
		{
			if ( item->column() == 1 )
			{
				const QString value = QInputDialog::getText(this, ui->filter_list->item(item->row(), 0)->text(), "Please enter a filter (ex., \"*.cpp; *.h\")", QLineEdit::Normal, item->text());

				if ( !value.isEmpty())
				{
					item->setText(value);
				}
			}
		}
	}
}
