#pragma once
#include "idevice.h"
#include <QObject>
#include <QList>
#include <QTimerEvent> 
#include "CModbus.h"

#define	RDM_TICKS	3		//max times for online check
#define RDM_TIMER	5000
#define DATETIME_TIMER	1000



class iReader;
class iDevice;
class iTag;
class iBC;
class iRDM : public QObject
{
	Q_OBJECT

public:
	static iRDM &Instance(QObject *parent = NULL)													//for singleton of iStation object
	{
		if (0 == _RDM)
		{
			_RDM = new iRDM(parent);
		}
		return *_RDM;
	}
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

public slots:
	void OnReloadRdmXml();

private:
	friend class iDevice;
	friend class iView;
	friend class iCfgDlg;
	friend class iBC;
	friend class iReader;

	iRDM(QObject *parent = NULL);																	//protected from external access
	static iRDM* _RDM;
	iReader*	reader;																				//RFID reader
	iDevice*	iotdevice;																			//IOT device
	CModbus *	modbus;
	iBC*		bc;

	QMap<quint64, iTag *> taglist;																	//<UID,tag>
	QMap<quint64, QString> tagOnline;																//<UID,epc>
	int			timerId;
	int         timer_datetime;
	int			RDM_ticks;

	QString		comName;

	//iot info
	QString		productkey;
	QString		devicename; 
	QString		devicesecret;
	QString		regionid;

	//modbus rtu parameters
	QString		modbustype;
	QString		rtuserial;
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
};
