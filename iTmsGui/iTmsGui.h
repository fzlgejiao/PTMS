#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_iTmsGui.h"


class iRdmView;
class iCfgPanel;
class iTagView;
class iTmsGui : public QMainWindow
{
	Q_OBJECT

public:
	iTmsGui(QWidget *parent = Q_NULLPTR);

private:
	Ui::iTmsGuiClass ui;
};
