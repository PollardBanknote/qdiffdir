#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include "mysettings.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    MySettings& settings = MySettings::instance();
    ui->diffToolLineEdit->setText(settings.getDiffToolSetting());
    ui->fileManagerLineEdit->setText(settings.getFileManagerSetting());
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_save_clicked()
{
    MySettings& settings = MySettings::instance();
    settings.setDiffTool(ui->diffToolLineEdit->text());
    settings.setFileManager(ui->fileManagerLineEdit->text());

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
