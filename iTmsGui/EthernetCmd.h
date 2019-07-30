#pragma once

#include <QObject>
#include <QHostAddress> 

#define UDP_IND		0xAA55

#define RemoteUdpPort	2900
#define RemoteTcpPort	2901

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

class QUdpSocket;
class QTcpSocket;
class QFile;
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

	void discoverRdm();
	void ipset(QString ipaddress);
	void downloadfile(QString filename);


signals:
	void fileSendready();
	void newRdmready(MSG_PKG & msg);

protected:
	bool	UDP_send(const MSG_PKG& msg);
	void	CMD_handle(const MSG_PKG& msg);

private slots:	
	void UDP_read();
	void onTcpError(QAbstractSocket::SocketError error);
	void onConnectedServer();
	void onConnectionstatechanged(QAbstractSocket::SocketState socketState);
	void onTcp_readyRead();
	void startTransfer();
	void updateClientProgress(qint64 bytes);

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
	QString filename;
	QFile  *sendfile;
	uint filesize;
	uint payload;
	uint filewritten;
	uint leftwritten;
	QByteArray sendblock;
	bool issendfile;
};
