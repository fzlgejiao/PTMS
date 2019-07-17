#pragma once

#include <QtWidgets/QDialog>
#include "ui_icfgdlg.h"

class iRDM;
class iCfgDlg : public QDialog
{
	Q_OBJECT

public:
	iCfgDlg(QWidget *parent = Q_NULLPTR);
	~iCfgDlg();

private:
	Ui::iCfgDlg ui;

	iRDM	&RDM;

public slots:
	void changePage(int row);
};
