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
	iTag(int sid,quint64 uid, const QString& epc,QObject *parent);
	~iTag();

	float parseTCode(ushort tcode);

private:
	friend class iRDM;
	friend class iReader;
	friend class iDevice;

	int				T_sid;																			//squence id of tag[1 - ], ordered by uid
	bool			T_available;																	//online/offline
	bool			T_updated;																		//data changed ,but not transmitted
	bool			T_alarm;																		//alarm or not(<online -> offline> ==> alarm)
	quint64			T_uid;																			//uid
	QString			T_epc;																			//epc 
	float			T_temp;																			//temperature
	CalibrationData T_caldata;
};
