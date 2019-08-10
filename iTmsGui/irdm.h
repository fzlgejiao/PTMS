#pragma once

#include <QObject>
#include <QMap>

class iTag :public QObject
{
	Q_OBJECT

public:
	iTag(QObject *parent = 0);
	~iTag();

private:
	friend class iRdm;

	int sid;
	quint64 UID;
	QString epc;
	qint8 rssi;
	quint8 oc_rssi;
	float temperature;
};

class iRdm :public QObject
{
	Q_OBJECT

public:
	iRdm(const QString &name, const QString &ip, const QString& mac, const QString& version, QObject *parent = 0);
	~iRdm();

private:
	friend class RdmModel;
	friend class iRdmView;
	friend class iCfgPanel;
	friend class iTagView;

	QString m_name;
	QString m_ip;
	QString m_MAC;
	QString m_Version;

	QMap<quint64, iTag *> assignedtaglist;
	QMap<quint64, iTag *> unassignedtaglist;
};

