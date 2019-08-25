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

bool TagAscendingbyEpc(iTag *tag1, iTag *tag2);

class iCfgPanel : public QTabWidget
{
	Q_OBJECT

public:
	iCfgPanel(QWidget *parent = Q_NULLPTR);
	~iCfgPanel();
	TagModel* Model() {	return model;}

private:
	Ui::iCfgPanel ui;

	TagModel *		model;
	EthernetCmd &	netcmd;
	QMap<int, QSerialPort::Parity> paritymap;
	QString			filename;

protected:
	bool saveRdmXml(iRdm *Rdm);

signals:
	void RdmModified();

public slots:
	void OnModbusParameters(MSG_PKG&);
	void OnTagsParaReady(MSG_PKG&);
	void OnTagEpc(MSG_PKG&);
	void OnRemoveTag();
	void OnEditTagLimit();
	void OnEditTagNote();
	void OnRdmSelected(iRdm *);
	void OnRdmSaved(iRdm *);
	void OnRdmDownloaded(iRdm *);
	void OnTagAdded(iTag *);
	void OnTagSelectChanged(const QModelIndex &index);
	void OnIoTParameters(MSG_PKG& msg);
	void OnRdmModified();
};
