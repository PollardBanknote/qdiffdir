#include "editmatchruledialog.h"
#include "ui_editmatchruledialog.h"

EditMatchRuleDialog::EditMatchRuleDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditMatchRuleDialog)
{
	ui->setupUi(this);
}

EditMatchRuleDialog::EditMatchRuleDialog(QString p, QString r, QString pcmd, QString rcmd, int w, QWidget* parent)
    : QDialog(parent),
    ui(new Ui::EditMatchRuleDialog)
{
	ui->setupUi(this);
	ui->pattern->setText(p);
	ui->replacement->setText(r);
	ui->pcommand->setText(pcmd);
	ui->rcommand->setText(rcmd);
	ui->weight->setValue(w);
}

EditMatchRuleDialog::~EditMatchRuleDialog()
{
	delete ui;
}

QString EditMatchRuleDialog::getPattern() const
{
	return ui->pattern->text();
}

QString EditMatchRuleDialog::getReplacement() const
{
	return ui->replacement->text();
}

QString EditMatchRuleDialog::getPatternCommand() const
{
	return ui->pcommand->text();
}

QString EditMatchRuleDialog::getReplacementCommand() const
{
	return ui->rcommand->text();
}

int EditMatchRuleDialog::getWeight() const
{
	return ui->weight->value();
}

void EditMatchRuleDialog::on_buttonBox_accepted()
{
	if (!ui->pattern->text().isEmpty() && !ui->replacement->text().isEmpty())
		accept();
}
