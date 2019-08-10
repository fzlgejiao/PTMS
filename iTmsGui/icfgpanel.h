#pragma once

#include <QTabWidget>
#include "ui_icfgpanel.h"
#include "EthernetCmd.h"

class QAbstractItemModel;
class iRdm;
class iTag;
class TagModel;
class iCfgPanel : public QTabWidget
{
	Q_OBJECT

public:
	iCfgPanel(QWidget *parent = Q_NULLPTR);
	~iCfgPanel();

private:
	Ui::iCfgPanel ui;

	TagModel *model;
	EthernetCmd &netcmd;


public slots:
	void OnModbusParameters(MSG_PKG&);
	void OnParaTagsFound(MSG_PKG&);
	void OnRemoveTag();
	void OnEditTag();
	void OnRdmSelected(iRdm *);
	void OnRdmSaved(iRdm *);
	void OnRdmDownloaded(iRdm *);
	void OnTagAdded(iTag *);
	void OnTagSelectChanged(const QModelIndex &index);

};
