#pragma once

#include <QTabWidget>
#include "ui_icfgpanel.h"
#include "EthernetCmd.h"
#include <QSerialPort>

class QAbstractItemModel;
class iRdm;
class iTag;
class TagModel;
class QSerialPort;
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
	QMap<int, QSerialPort::Parity> paritymap;

protected:
	bool saveRdmXml(iRdm *Rdm);


public slots:
	void OnModbusParameters(MSG_PKG&);
	void OnTagsParaReady(MSG_PKG&);
	void OnTagEpc(MSG_PKG&);
	void OnRemoveTag();
	void OnEditTag();
	void OnRdmSelected(iRdm *);
	void OnRdmSaved(iRdm *);
	void OnRdmDownloaded(iRdm *);
	void OnTagAdded(iTag *);
	void OnTagSelectChanged(const QModelIndex &index);
	void OnIoTParameters(MSG_PKG& msg);

};
