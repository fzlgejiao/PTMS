#pragma once

#include <QObject>
#include <QMap>

#define		RDM_NAME_NOTE	15
#define		TAG_EPC_SIZE	14																		//epc size should be even, and limited below 16
#define		TAG_NOTE_SIZE	15

class iTag :public QObject
{
	Q_OBJECT

public:
	iTag(quint64 uid,const QString& epc,QObject *parent = 0);
	iTag(const iTag& tag);
	~iTag();
	quint64 uid() { return t_uid; }
	QString epc() { return t_epc; }

private:
	friend class iRdm;
	friend class TagModel;
	friend class iCfgPanel;
	friend class iDataView;
	friend class EthernetCmd;

	int		t_sid;
	quint64 t_uid;
	QString t_epc;
	qint8	t_rssi;
	quint8	t_oc_rssi;
	float	t_temperature;
	QString t_note;
	quint8	t_uplimit;
	quint8	t_alarm;
	quint8	t_online;
};

class iRdm :public QObject
{
	Q_OBJECT

public:
	iRdm(const QString &name, const QString &ip, const QString& mac, const QString& version, const QString& note, QObject *parent = 0);
	~iRdm();
	bool isModified() { return m_bModified; }
	void setModified(bool bModified) { m_bModified = bModified; }

private:
	friend class RdmModel;
	friend class iRdmView;
	friend class iCfgPanel;
	friend class iDataView;
	friend class EthernetCmd;

	QString m_name;
	QString m_ip;
	QString m_MAC;
	QString m_Version;
	QString m_note;
	QString m_comname;

	bool	m_bModified;
};

class RdmModel;
class TagModel;
class iSys :public QObject
{
	Q_OBJECT

public:
	static iSys & Instance()
	{
		static iSys _iSys;
		return _iSys;
	}
	~iSys() {}

	RdmModel	*rdmModel;
	TagModel	*tagModelOnline;
	TagModel	*tagModelPara;
	TagModel	*tagModelData;
private:
	iSys(QObject *parent=0)
		: QObject(parent)
	{
		rdmModel = NULL;
		tagModelOnline = NULL;
		tagModelPara = NULL;
		tagModelData = NULL;

	}
};