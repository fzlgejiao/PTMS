#pragma once

#include <QWidget>
#include "ui_irdmview.h"
#include "EthernetCmd.h"

class QStandardItemModel;
class EthernetCmd;
class iRdmView : public QWidget
{
	Q_OBJECT

public:
	iRdmView(QWidget *parent = Q_NULLPTR);
	~iRdmView();

private slots:
	void onbtndiscover();
	void onbtnDownload();
	void NewRdmfound(MSG_PKG &msg);
	
private:
	Ui::iRdmView ui;
	EthernetCmd &m_Enetcmd;
	QStandardItemModel *model;	
};
