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
	void mouseDoubleClickEvent(QMouseEvent * event);

private:
	Ui::iTile ui;

	iTag*	_tag;

signals:
	void tileDBClicked(iTile *);
public slots:
	void OnDataChanged();
};
