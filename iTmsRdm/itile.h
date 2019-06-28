#pragma once

#include <QtWidgets/QWidget> 
#include "ui_itile.h"

class iTag;
class iTile : public QWidget
{
	Q_OBJECT

public:
	iTile(iTag* tag,QWidget *parent = Q_NULLPTR);
	~iTile();
protected:
	void paintEvent(QPaintEvent *event);

private:
	Ui::iTile ui;

	iTag*	_tag;
};
