#pragma once

#include <QObject>
#include <QHostAddress> 
#include <QTcpSocket>

#define UDP_IND			0xAA55
#define UDP_DATA		1200
#define RemoteUdpPort	2900
#define RemoteTcpPort	2901

#define	TAG_NUM			16

typedef enum {
	XmlFile = 0,
	TarFile,
	Invalid
}FileType;

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
		uchar			data[UDP_DATA];
	}cmd_pkg;
}MSG_PKG;

typedef enum {
	UDP_DISCOVER			= 1,
	UDP_ONLINE				= 2,
	UDP_READMODBUSSETTING	= 3,
	UDP_READIOTSETTING		= 4,
	UDP_READTAGSSETTING		= 5,
	UDP_READTAGSONLINE		= 6,
	UDP_READTAGSDATA		= 7,
	UDP_SETTAGEPC			= 8,
	UDP_SETRDMIP			= 9,
	UDP_FILEPARAMETER		= 0x10
}UDP_CMD;


typedef struct {
	FileType filetype;
	quint8	reserved[3];
	quint32	filesize;
	char	filename[16];
}File_Paramters;

typedef struct {
	char	RdmName[16];
	char	RdmIp[16];
	char	RdmMAC[32];
	char	RdmVersion[8];
	char	RdmNote[16];
	char	RdmOrg[16];
	char	RdmComName[32];
}RDM_Paramters;

typedef struct {
	quint8	type;				//0--->RTU mode,1---->TCP mode ,others invalid
	quint8	reserved;
	quint16	tcp_port;			//maybe only can be read
	char	rtu_comname[16];
	quint32	rtu_baudrate;
	quint8	rtu_address;
	quint8	rtu_parity;
	quint16 reserved1;
}MODBUS_Paramters;

typedef struct {
	char	productkey[16];
	char	devicename[16];
	char	devicesecret[48];
	char	regionid[16];
}IOT_Paramters;

typedef struct {
	struct {
		quint8	tagcount;
		quint8	reserved1;
		quint16	reserved2;
	}Header;
	struct {
		quint64 uid;
		quint8	sid;
		quint8	upperlimit;
		quint16 reserved;
		char	name[16];
		char	note[16];
	}Tags[TAG_NUM];
}Tags_Parameters;

typedef struct {
	struct {
		quint8	tagcount;
		quint8	reserved1;
		quint16	reserved2;
	}Header;
	struct {
		quint64 uid;
		quint8	sid;
		quint8	alarm;
		qint8	rssi;
		quint8	oc_rssi;
		quint16 temperature;		//temperature is a float ,so real temperature= temperature *0.1 ¡æ
		quint8	online;				//0: offline, 1: online
		quint8	reserved;
		char	name[16];
		char	note[16];
	}Tags[TAG_NUM];
}Tags_Data;

typedef struct {
	struct {
		quint8	tagcount;
		quint8	reserved1;
		quint16	reserved2;
	}Header;
	struct {
		quint64 uid;
		char	name[16];
	}Tags[TAG_NUM];
}Tags_Online;

typedef struct {
	quint64 uid;
	char	epc[16];
}Tag_epc;

typedef enum {
	Start=0,
	Sending,
	Finished,
	TransferError,
	StateNONE
}FileTransferState;

typedef struct {
	char	mac[32];
	char	ip[16];
}IpSetFormat;

class QUdpSocket;
class QFile;
class iRdm;
class iTag;
class EthernetCmd : public QObject
{
	Q_OBJECT

public:
	EthernetCmd(QObject *parent = 0);
	~EthernetCmd();

	static EthernetCmd & Instance()
	{
		static EthernetCmd ethernet_cmd;
		return ethernet_cmd;
	}

	void UDP_discoverRdm();
	void UDP_get_modbusparameters(iRdm* rdm);
	void UDP_get_iotparameters(iRdm* rdm);
	void UDP_get_tagsonline(iRdm* rdm);
	void UDP_get_tagspara(iRdm* rdm);
	void UDP_get_tagsdata(iRdm* rdm);
	void UDP_set_tagepc(iRdm* rdm,iTag* tag);
	void UDP_ipset(iRdm *rdm);
	void UDP_fileinfo(iRdm* rdm,QString filename, FileType type);

	QString errorstring() { return tcpClient->errorString(); }

protected:
	bool UDP_send(const MSG_PKG& msg);
	void CMD_handle(const MSG_PKG& msg);



private:
	//udp socket
	QUdpSocket  *udpSocket;
	MSG_PKG		RxMsg;
	int         remoteUdpPort;

	//tcp socket
	QTcpSocket  *tcpClient;
	int         remoteTcpPort;
	bool		isconnected;

	//file info
	QString		filename;
	QFile*		sendfile;
	uint		filesize;
	uint		payload;
	uint		filewritten;
	uint		leftwritten;
	QByteArray	sendblock;
	bool		issendfile;

signals:
	void newRdmReady(MSG_PKG & msg);
	void RdmParaReady(MSG_PKG& msg);
	void ModbusParamReady(MSG_PKG& msg);
	void IotParaReady(MSG_PKG& msg);
	void TagsOnlineReady(MSG_PKG& msg);
	void TagsDataReady(MSG_PKG& msg);
	void TagsParaReady(MSG_PKG& msg);
	void TagEpcReady(MSG_PKG& msg);																	//write tag epc acked
	void sendprogress(int value, int max);
	void transferstate(FileTransferState state);

private slots:	
	void UDP_read();
	void TCP_Error(QAbstractSocket::SocketError error);
	void TCP_SocketStateChanged(QAbstractSocket::SocketState socketState);
	void startTransfer();
	void updateClientProgress(qint64 bytes);
};
