#pragma once

#include <QTabWidget>
#include "ui_icfgpanel.h"
#include "EthernetCmd.h"

class QAbstractItemModel;
class iRdm;
class iCfgPanel : public QTabWidget
{
	Q_OBJECT

public:
	iCfgPanel(QWidget *parent = Q_NULLPTR);
	~iCfgPanel();

private:
	Ui::iCfgPanel ui;

	QAbstractItemModel *model;
	EthernetCmd &netcmd;


public slots:
	void OnModbusParameters(MSG_PKG&);
	void OnRemoveTag();
	void OnRdmSelected(iRdm *);
};
