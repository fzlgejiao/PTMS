#pragma once

#include <QTabWidget>
#include "ui_idataview.h"
#include "EthernetCmd.h"

class QAbstractItemModel;
class iRdm;
class TagModel;
class iTag;
class iSys;
class iDataView : public QTabWidget
{
	Q_OBJECT

public:
	iDataView(QWidget *parent = Q_NULLPTR);
	~iDataView();

private:
	Ui::iDataView ui;
	iSys&			oSys;
	TagModel *		tagModel;
	EthernetCmd &	netcmd;

public slots:
	void OnRdmSelected(iRdm *);
	void OnMsgTagsDataReady(MSG_PKG&);
	void OnMsgTagEpc(MSG_PKG&);
};
