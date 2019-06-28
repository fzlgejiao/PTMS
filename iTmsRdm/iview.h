#pragma once

#include <QtWidgets/QWidget> 
#include "ui_iview.h"
#include "irdm.h"

class iTile;
class iView : public QWidget
{
	Q_OBJECT

public:
	iView(QWidget *parent = Q_NULLPTR);
	~iView();
protected:
	void paintEvent(QPaintEvent *event);

private:
	Ui::iView ui;
	iRDM	rdm;
	iTile	*tiles[6];
};
