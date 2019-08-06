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

protected:
	virtual void timerEvent(QTimerEvent *event);
	QString currentRdm();

private slots:
	void onbtndiscover();
	void onbtnDownload();
	void NewRdmfound(MSG_PKG &msg);
	void OnRdmSelectChanged();
	
private:
	Ui::iRdmView ui;
	EthernetCmd &m_Enetcmd;

	int		m_n2sTimerId;
};
