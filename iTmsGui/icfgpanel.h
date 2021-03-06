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
class iSys;
bool TagAscendingbyEpc(iTag *tag1, iTag *tag2);

class iCfgPanel : public QTabWidget
{
	Q_OBJECT

public:
	iCfgPanel(QWidget *parent = Q_NULLPTR);
	~iCfgPanel();
	TagModel* Model() {	return tagModel;}

private:
	Ui::iCfgPanel ui;
	iSys&			oSys;
	TagModel *		tagModel;
	EthernetCmd &	netcmd;
	QMap<int, QSerialPort::Parity> paritymap;
	QString			filename;

protected:
	bool saveRdmXml(iRdm *Rdm);

signals:
	void RdmModified();

public slots:
	void OnRemoveTag();
	void OnEditTagLimit();
	void OnEditTagNote();

	void OnRdmSelected(iRdm *);
	void OnRdmSaved(iRdm *);
	void OnRdmDownloaded(iRdm *);
	void OnTagAdded(iTag *);
	void OnTagSelectChanged(const QModelIndex &index);
	void OnRdmModified();
	void Ontextchanged(QString text);

	void OnMsgModbusParameters(MSG_PKG&);
	void OnMsgTagsParaReady(MSG_PKG&);
	void OnMsgTagEpc(MSG_PKG&);
	void OnMsgIoTParameters(MSG_PKG& msg);

};
