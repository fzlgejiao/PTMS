#pragma once
#include "idevice.h"
#include <QObject>
#include <QList>
#include <QTimerEvent> 


#define	RDM_TICKS		3		//max times for online check
#define IOT_TIMER		5000
#define RDM_TIMER		2000
#define DATETIME_TIMER	1000

enum HWVER {
	HW_V1 = 0,
	HW_V2,
	HW_V3
};


class iReader;
class iDevice;
class iTag;
class iBC;
class CModbus;
class iLed;
class iRDM : public QObject
{
	Q_OBJECT

public:
	iRDM(QObject *parent = NULL);																	
	~iRDM();

	static  void ERR_msg(const QString& module,const QString& error);

	iTag*	Tag_get(quint64 uid) {return  taglist.value(uid, NULL);}
	iTag*	Tag_getbysid(int sid);
	int		Tag_count() { return taglist.count(); }

	CModbus* getCModbus() { return modbus; }

	void    Tmr_stop() { this->killTimer(tmrRDM);  this->killTimer(tmrTime); this->killTimer(tmrIOT);}
	void    Tmr_start() { tmrRDM = this->startTimer(RDM_TIMER); tmrTime = this->startTimer(DATETIME_TIMER); tmrIOT = this->startTimer(IOT_TIMER);
	}

protected:

	bool	Cfg_load(const QString& xml);															//load rdm configuration from xml file
	void	Cfg_readrdm(QXmlStreamReader& xmlReader);
	void	Cfg_readcfg(QXmlStreamReader& xmlReader);
	void	Cfg_readtags(QXmlStreamReader& xmlReader);
	void	Cfg_skipUnknownElement(QXmlStreamReader& xmlReader);

	iTag*	Tag_add(int sid,quint64 uid,const QString& epc);
	int		HW_ver();

	virtual void timerEvent(QTimerEvent *event);

private:
	friend class iDevice;
	friend class iView;
	friend class iCfgDlg;
	friend class iBC;
	friend class iReader;
	friend class CModbus;

	iReader*	reader;																				//RFID reader
	iDevice*	iotdevice;																			//IOT device
	CModbus *	modbus;
	iBC*		bc;
	iLed*		led;

	QMap<quint64, iTag *> taglist;																	//<UID,tag>
	QMap<quint64, QByteArray> tagOnline;															//<UID,epc>
	int			tmrRDM;
	int         tmrTime;
	int         tmrIOT;
	int			RDM_ticks;


	//iot info
	QString		productkey;
	QString		devicename; 
	QString		devicesecret;
	QString		regionid;

	//modbus rtu parameters
	QString		modbustype;
	QString		rtucomname;
	int			rtuslaveaddress;
	int			rtubaudrate;
	int			rtuparity;
	//modbus tcp parameters
	int			TcpPort;
	
	bool		RDM_available;
	bool		RDM_alarm;

	//RDM info
	QString		RDM_mac;
	QString		RDM_name;
	QString		RDM_ip;
	QString		RDM_note;
	QString		RDM_comname;

public slots:
	void RDM_init();

signals:
	void cfgChanged();
	void tagLost(iTag *);
};
