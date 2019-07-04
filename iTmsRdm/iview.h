#pragma once

#include <QtWidgets/QWidget> 
#include "ui_iview.h"

class iTile;
class iRDM;
class iTag;
class iView : public QWidget
{
	Q_OBJECT

public:
	iView(iRDM* rdm,QWidget *parent = Q_NULLPTR);
	~iView();
	iTile* Tile(int sid) { return tilelist.value(sid, NULL); }

protected:
	void paintEvent(QPaintEvent *event);

private:
	Ui::iView ui;
	iRDM	*RDM;
	QMap<int, iTile*> tilelist;																		//<tag_sid,iTile *>


public slots:
	void OnTagUpdated(iTag*);
};
