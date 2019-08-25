#pragma once
#include "idevice.h"
#include <QObject>
#include <QList>
#include <QTimerEvent> 


#define	RDM_TICKS	3		//max times for online check
#define RDM_TIMER	5000
#define DATETIME_TIMER	1000



class iReader;
class iDevice;
class iTag;
class iBC;
class CModbus;
class iRDM : public QObject
{
	Q_OBJECT

public:
	iRDM(QObject *parent = NULL);																	
	~iRDM();

	iTag*	Tag_get(quint64 uid) {return  taglist.value(uid, NULL);}
	iTag*	Tag_getbysid(int sid);
	int		Tag_count() { return taglist.count(); }

	void    Tmr_stop() { this->killTimer(timerId);  this->killTimer(timer_datetime);}
	void    Tmr_start() { timerId = this->startTimer(RDM_TIMER);timer_datetime= this->startTimer(DATETIME_TIMER);}

protected:

	bool	Cfg_load(const QString& xml);															//load rdm configuration from xml file
	void	Cfg_readrdm(QXmlStreamReader& xmlReader);
	void	Cfg_readcfg(QXmlStreamReader& xmlReader);
	void	Cfg_readtags(QXmlStreamReader& xmlReader);
	void	Cfg_skipUnknownElement(QXmlStreamReader& xmlReader);

	iTag*	Tag_add(int sid,quint64 uid,const QString& epc);

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

	QMap<quint64, iTag *> taglist;																	//<UID,tag>
	QMap<quint64, QString> tagOnline;																//<UID,epc>
	int			timerId;
	int         timer_datetime;
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
};
