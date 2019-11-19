#pragma once

#include <QTabWidget>
#include "ui_idataview.h"
#include "EthernetCmd.h"

class QAbstractItemModel;
class iRdm;
class TagModel;
class iTag;
class iSys;
class QSqlTableModel;
class iDataView : public QTabWidget
{
	Q_OBJECT

public:
	iDataView(QWidget *parent = Q_NULLPTR);
	~iDataView();

protected:
	virtual void timerEvent(QTimerEvent *event);
private:
	Ui::iDataView ui;
	iSys&			oSys;
	TagModel *		tagModel;
	EthernetCmd &	netcmd;
	QSqlTableModel*	tagModelOld;
	int				m_n5mTimerId;

public slots:
	void OnRdmSelected(iRdm *);
	void OnMsgTagsDataReady(MSG_PKG&);
	void OnMsgTagEpc(MSG_PKG&);
	void OnDeleteData();
	void OnClearData();
	void OnTagsOldSelected();
};
