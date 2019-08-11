#pragma once

#include <QObject>
#include <QMap>

class iTag :public QObject
{
	Q_OBJECT

public:
	iTag(quint64 uid,const QString& epc,QObject *parent = 0);
	iTag(const iTag& tag);
	~iTag();
	quint64 uid() { return t_uid; }

private:
	friend class iRdm;
	friend class TagModel;
	friend class iCfgPanel;
	friend class iTagView;
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
};

class iRdm :public QObject
{
	Q_OBJECT

public:
	iRdm(const QString &name, const QString &ip, const QString& mac, const QString& version, const QString& note, QObject *parent = 0);
	~iRdm();

private:
	friend class RdmModel;
	friend class iRdmView;
	friend class iCfgPanel;
	friend class iTagView;
	friend class EthernetCmd;

	QString m_name;
	QString m_ip;
	QString m_MAC;
	QString m_Version;
	QString m_note;
};

