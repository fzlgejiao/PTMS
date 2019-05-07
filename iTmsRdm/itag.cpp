#include "itag.h"

iTag::iTag(int sid, quint64 uid, const QString& epc, QObject *parent)
	: QObject(parent)
{
	T_available	= false;
	T_sid = sid;
	T_uid = uid;
	T_epc = epc;
	T_caldata.all= 0;
	T_temp= 0;
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