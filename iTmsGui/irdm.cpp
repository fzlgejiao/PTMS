#include "irdm.h"

iRdm::iRdm(const QString &name, const QString &ip, const QString& mac, const QString& version, const QString& note, QObject *parent)
	: QObject(parent)
{
	m_name = name;
	m_ip = ip;
	m_MAC = mac;
	m_Version = version;
	m_note = note;
	m_comname = "";
}

iRdm::~iRdm()
{
}



iTag::iTag(quint64 uid, const QString& epc,QObject *parent)
	: QObject(parent),t_uid(uid),t_epc(epc)
{
	t_sid = 0;
	t_rssi = 0;
	t_oc_rssi = 0;
	t_temperature = 0.;
	t_uplimit = 70;
	t_alarm = 0;
}
iTag::iTag(const iTag& tag)
{
	t_sid = tag.t_sid;
	t_uid = tag.t_uid;
	t_epc = tag.t_epc;
	t_rssi = tag.t_rssi;
	t_oc_rssi = tag.t_oc_rssi;
	t_temperature = tag.t_temperature;
	t_note = tag.t_note;
	t_uplimit = tag.t_uplimit;
	t_alarm = tag.t_alarm;
}
iTag::~iTag()
{
}