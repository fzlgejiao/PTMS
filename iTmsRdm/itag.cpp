#include "itag.h"

iTag::iTag(int sid, quint64 uid, const QString& epc, QObject *parent)
	: QObject(parent)
{
	T_enable = true;
	T_ticks	= 0;
	T_sid = sid;
	T_uid = uid;
	T_epc = epc;
	T_rssi = -99;
	T_OC_rssi = -99;
	T_caldata.all= 0;
	T_temp= -99;
	T_alarm_offline = false;
	T_alarm_temperature = false;
	T_uplimit = TAG_T_MAX;

	T_data_flag = 0;
	T_event_flag = 0;
}

iTag::~iTag()
{
}
float iTag::parseTCode(ushort tcode)
{
	float k = (float)(T_caldata.bits.temp2 - T_caldata.bits.temp1) / (T_caldata.bits.code2 - T_caldata.bits.code1);
	short delta = tcode - T_caldata.bits.code1;

	return (0.1f*(k * delta + T_caldata.bits.temp1 - 800));
}
bool	iTag::hasDataFlag(ushort flag)
{
	if (T_data_flag & flag)
	{
		T_data_flag &= ~flag;
		return true;
	}
	return false;
}
