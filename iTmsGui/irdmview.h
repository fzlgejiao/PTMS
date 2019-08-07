#pragma once

#include <QWidget>
#include "ui_irdmview.h"
#include "EthernetCmd.h"

class EthernetCmd;
class CRdm;
class RdmModel;

class iRdmView : public QWidget
{
	Q_OBJECT

public:
	iRdmView(QWidget *parent = Q_NULLPTR);
	~iRdmView();

protected:
	virtual void timerEvent(QTimerEvent *event);


private slots:
	void onbtndiscover();
	void onbtnDownload();
	void NewRdmfound(MSG_PKG &msg);
	void OnRdmSelectChanged(const QModelIndex & index);
	
private:
	Ui::iRdmView ui;
	EthernetCmd &m_Enetcmd;
	RdmModel *rdmmodel;


	int		m_n2sTimerId;
};
