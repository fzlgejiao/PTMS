#pragma once

#include <QWidget>
#include "ui_irdmview.h"
#include "EthernetCmd.h"

class EthernetCmd;
class CRdm;
class RdmModel;
class QSortFilterProxyModel;
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
	void OnRdmTableActived(const QModelIndex &index);
	
	inline bool hasRdm(QString mac) { return rdmsmap.value(mac, false); }

private:
	Ui::iRdmView ui;
	EthernetCmd &m_Enetcmd;
	RdmModel *Rawrdmmodel;
	QSortFilterProxyModel *rdmmodel;
	QMap<QString, CRdm *> rdmsmap;
};
