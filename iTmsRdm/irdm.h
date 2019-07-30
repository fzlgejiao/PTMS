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

#define UdpLocalPort	2900
#define TcpServerPort	2901

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
	UDP_ONLINE = 2,
	UDP_READMODBUSSETTING = 3,
	UDP_READIOTSETTING = 4,
	UDP_READTAGS = 5,
	UDP_SETIP = 6,
	UDP_FILEPARAMETER = 0x10
}UDP_CMD;


typedef struct {
	quint32	filesize;
	char	filename[16];
	char	savedpath[128];
	quint16	fileAttribute;
}File_Paramters;

typedef struct {
	char	RdmName[16];
	char	RdmIp[16];
	char	RdmMAC[32];
	char	RdmVersion[8];
	char	RdmNote[16];
	char	RdmOrg[16];
}RDM_Paramters;

typedef struct {
	quint8	type;				//0--->RTU mode,1---->TCP mode ,others invalid
	char	comname[16];
	quint32	baudrate;
	quint8	slaveaddress;
	quint8	parity;
	quint16	tcpport;			//maybe only can be read
	quint8	reserved;
}MODBUS_Paramters;

typedef struct {
	char	productkey[16];
	char	devicename[16];
	char	devicesecret[48];
	char	regionid[16];
}IOT_Paramters;

typedef struct {
	quint8	tagcount;
	quint8	reserved1;
	quint16	reserved2;
}Tag_Data_Header;

typedef struct {
	quint8	sid;
	quint64 uid;
	quint8	upperlimit;
	qint8	rssi;
	quint8	oc_rssi;
	quint16 temperature;		//temperature is a float ,so real temperature= temperature *0.1 ¡æ
}Tag_Data;


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
	void	CMD_handle(const MSG_PKG& msg);
	bool	TCP_send(const MSG_PKG& msg);

	


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

	//udp socket
	QUdpSocket  *udpSocket;
	MSG_PKG		RxMsg;
	//Tcp server 
	QTcpServer m_tcpServer;
	QTcpSocket *m_tcpServerConnection;

	//File info
	QFile		*savedfile;
	int			recvbytes;
	int			totalbytes;
	QString     filename;
	int			filesize;
	int			fileReceived;
	QByteArray  fileblock;

public slots:
	void UDP_read();
	void TcpServer_readyRead();
	void acceptConnection();
	void onTcpError(QAbstractSocket::SocketError error);
	void onTcpSocketstatechanged(QAbstractSocket::SocketState state);

signals:
	void finished();
	void receivedprogress(qint64 recevied,qint64 total);

};
