#pragma once
#include "idevice.h"
#include <QObject>
#include <QList>
#include <QTimerEvent> 
#include "CModbus.h"

#define	RDM_TICKS	3		//max times for online check
#define RDM_TIMER	5000
#define DATETIME_TIMER	1000

#define UDP_IND		0xAA55
/* command packet and message */
typedef struct {
	ushort ind;                         // cmd identifier: 0xAA55 
	uchar  cmd;
	uchar  len;
}CMD_HEADER;

typedef struct {
	QHostAddress	rIP;
	quint16			rPort;
	struct 
	{
		CMD_HEADER		header;
		uchar			data[256];
	}cmd_pkg;
}MSG_PKG;

typedef enum {
	UDP_DISCOVER = 1,
}UDP_CMD;

class iReader;
class iDevice;
class iTag;
class QUdpSocket;
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

	bool	UDP_send(const MSG_PKG& msg);
	void	UDP_handle(const MSG_PKG& msg);

private:
	friend class iDevice;
	friend class iView;
	friend class iCfgDlg;

	iRDM(QObject *parent = NULL);																	//protected from external access
	static iRDM* _RDM;
	iReader*	reader;																				//RFID reader
	iDevice*	iotdevice;																			//IOT device
	CModbus *	modbus;

	QMap<quint64, iTag *> taglist;																	//tag's ID string map to tag
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
	QString  modbustype;
	QString  rtuserial;
	int		 rtuslaveaddress;
	int      rtubaudrate;
	int      rtuparity;
	//modbus tcp parameters
	int		 TcpPort;
	
	bool		RDM_available;
	bool		RDM_alarm;

	//RDM info
	QString		RDM_mac;
	QString		RDM_name;
	QString		RDM_ip;

	//udp/tcp socket
	QUdpSocket  *udpSocket;
	MSG_PKG		RxMsg;

public slots:
	void UDP_read();
};
