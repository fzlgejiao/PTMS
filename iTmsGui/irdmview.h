#pragma once

#include <QWidget>
#include "ui_irdmview.h"
#include "EthernetCmd.h"

class EthernetCmd;
class iRdm;
class iTag;
class RdmModel;
class TagModel;
class iSys;
class iRdmView : public QWidget
{
	Q_OBJECT

public:
	iRdmView(QWidget *parent = Q_NULLPTR);
	~iRdmView();
	iRdm* selectedRdm();

protected:
	virtual void timerEvent(QTimerEvent *event);
	iTag* selectedTag();


private slots:
	void onbtndiscover();
	void onbtnDownload();
	void onbtnChangeIP();
	void OnbtnFindTags();
	void onbtnChangeEpc();
	void onbtnAddToSys();
	void onbtnUpgrade();

	void OnRdmSelectChanged(const QModelIndex & index);
	void OnTagSelectChanged(const QModelIndex & index);
	void OnTagDataChanged(const QModelIndex &);
	void OnTagDataFailed(const QModelIndex &index, const QString& error);
	void OnRdmModified();
	void OnRdmIpChanged(iRdm *rdm);

	void OnMsgRdmfound(MSG_PKG &msg);
	void OnMsgOnlineTagsFound(MSG_PKG &msg);
	void OnMsgTagEpc(MSG_PKG&);
	
private:
	Ui::iRdmView ui;
	iSys&		oSys;
	EthernetCmd &netcmd;
	RdmModel*	rdmModel;
	TagModel*	tagModel;


	int		m_n2sTimerId;

signals:
	void RdmSelected(iRdm *);
	void RdmSaved(iRdm *);
	void RdmDownloaded(iRdm *);
	void tagAdded(iTag *);
};
