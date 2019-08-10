#pragma once

#include <QWidget>
#include "ui_irdmview.h"
#include "EthernetCmd.h"

class EthernetCmd;
class iRdm;
class iTag;
class RdmModel;
class TagModel;
class iRdmView : public QWidget
{
	Q_OBJECT

public:
	iRdmView(QWidget *parent = Q_NULLPTR);
	~iRdmView();

protected:
	virtual void timerEvent(QTimerEvent *event);
	iRdm* selectedRdm();
	iTag* selectedTag();


private slots:
	void onbtndiscover();
	void onbtnDownload();
	void OnbtnFindTags();
	void onbtnChangeEpc();
	void onbtnAddToSys();
	void NewRdmfound(MSG_PKG &msg);
	void OnlineTagsFound(MSG_PKG &msg);
	void OnRdmSelectChanged(const QModelIndex & index);
	void OnTagSelectChanged(const QModelIndex & index);
	
private:
	Ui::iRdmView ui;
	EthernetCmd &m_Enetcmd;
	RdmModel*	rdmmodel;
	TagModel*	tagModel;


	int		m_n2sTimerId;

signals:
	void RdmSelected(iRdm *);
	void RdmSaved(iRdm *);
	void RdmDownloaded(iRdm *);
	void tagAdded(iTag *);
};
