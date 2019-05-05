#pragma once

#include <QObject>

typedef union
{
	struct
	{
		quint64 version : 2;
		quint64 temp2 : 11;
		quint64 code2 : 12;
		quint64 temp1 : 11;
		quint64 code1 : 12;
		quint64 crc : 16;
	}bits;
	quint64 all;
}CalibrationData;

class iTag : public QObject
{
	Q_OBJECT

public:
	iTag(quint64 tag_ID, QObject *parent);
	~iTag();

	float parseTCode(ushort tcode);

private:
	friend class iRDM;
	friend class iReader;
	friend class iDevice;

	quint64			T_id;
	QString			T_epc;
	float			T_temp;																			//temperature
	CalibrationData T_caldata;
};
