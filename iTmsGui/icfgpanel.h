#pragma once

#include <QTabWidget>
#include "ui_icfgpanel.h"

class QAbstractItemModel;
class iCfgPanel : public QTabWidget
{
	Q_OBJECT

public:
	iCfgPanel(QWidget *parent = Q_NULLPTR);
	~iCfgPanel();

private:
	Ui::iCfgPanel ui;

	QAbstractItemModel *model;
};
