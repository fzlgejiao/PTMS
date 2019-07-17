#pragma once

#include <QDialog>
#include "ui_addtagdlg.h"

class AddTagDlg : public QDialog
{
	Q_OBJECT

public:
	AddTagDlg(QWidget *parent = Q_NULLPTR);
	~AddTagDlg();

private:
	Ui::AddTagDlg ui;
};
