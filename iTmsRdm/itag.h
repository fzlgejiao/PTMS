#pragma once

#include <QObject>

#define	TAG_TICKS	3		//max times for online check		

enum TagDataFlag {
	Tag_Temperature = 0x0001,
	Tag_UID			= 0x0002,
	Tag_EPC			= 0x0004,
	Tag_Upperlimit	= 0x0008,
	Tag_Online		= 0x0010,
	Tag_Switch		= 0x0020

};

typedef union
{
	struct
	{
		quint64 version : 2;
		quint64 temp2	: 11;
		quint64 code2	: 12;
		quint64 temp1	: 11;
		quint64 code1	: 12;
		quint64 crc		: 16;
	}bits;
	quint64 all;
}CalibrationData;



class iTag : public QObject
{
	Q_OBJECT

public:
	iTag(int sid,quint64 uid, const QString& epc,QObject *parent);
	~iTag();
	bool	isonline() { return T_ticks > 0; }
	float	parseTCode(ushort tcode);
	bool	hasDataFlag(ushort flag);

private:
	friend class iRDM;
	friend class iReader;
	friend class iDevice;

	bool			T_enable;
	int				T_sid;																			//squence id of tag[1 - ], ordered by uid
	int				T_ticks;																		//ticks to check online(0: offline, >0: online)	
	bool			T_updated;																		//data changed ,but not transmitted
	bool			T_alarm_offline;																//offline alarm or not(<online -> offline> ==> alarm)
	bool			T_alarm_temperature;															//temperature out of range alarm
	quint64			T_uid;																			//uid
	QString			T_epc;																			//epc 
	float			T_temp;																			//temperature
	int				T_uplimit;																		//up limit for temperature
	CalibrationData T_caldata;

	ushort			T_data_flag;
	ushort			T_event_flag;
};
