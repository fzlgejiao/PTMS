#pragma once

#include <QWidget>
#include "ui_irdmview.h"

class QAbstractItemModel;
class iRdmView : public QWidget
{
	Q_OBJECT

public:
	iRdmView(QWidget *parent = Q_NULLPTR);
	~iRdmView();

private:
	Ui::iRdmView ui;

	QAbstractItemModel *model;
};
