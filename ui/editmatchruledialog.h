#ifndef EDITMATCHRULEDIALOG_H
#define EDITMATCHRULEDIALOG_H

#include <QDialog>

namespace Ui {
class EditMatchRuleDialog;
}

class EditMatchRuleDialog : public QDialog
{
	Q_OBJECT

public:
	explicit EditMatchRuleDialog(QWidget *parent = 0);
	EditMatchRuleDialog(QString, QString, QString, QString, int, QWidget* parent = 0);
	~EditMatchRuleDialog();

	QString getPattern() const;
	QString getReplacement() const;
	QString getPatternCommand() const;
	QString getReplacementCommand() const;
	int getWeight() const;
private slots:
	void on_buttonBox_accepted();

private:
	Ui::EditMatchRuleDialog *ui;
};

#endif // EDITMATCHRULEDIALOG_H
