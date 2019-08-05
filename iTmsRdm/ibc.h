#pragma once

#include <QObject>
#include <QtNetwork/QUdpSocket>

#define UDP_IND		0xAA55

#define UDP_PORT	2900
#define TCP_PORT	2901

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
	UDP_DISCOVER			= 1,
	UDP_ONLINE				= 2,
	UDP_READMODBUSSETTING	= 3,
	UDP_READIOTSETTING		= 4,
	UDP_READTAGS			= 5,
	UDP_SETIP				= 6,
	UDP_FILEPARAMETER		= 0x10
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
	quint8	reserved;
	quint16	tcp_port;			//maybe only can be read
	char	rtu_comname[16];
	quint32	rtu_baudrate;
	quint8	rtu_address;
	quint8	rtu_parity;
}MODBUS_Paramters;

typedef struct {
	char	productkey[16];
	char	devicename[16];
	char	devicesecret[48];
	char	regionid[16];
}IOT_Paramters;

typedef struct {
	quint8	tagcount;
	quint8	reserved[3];
}Tag_Data_Header;

typedef struct {
	quint64 uid;
	quint8	sid;
	quint8	upperlimit;
	qint8	rssi;
	quint8	oc_rssi;
	quint16 temperature;		//temperature is a float ,so real temperature= temperature *0.1 ¡æ
	quint8	reserved[2];
}Tag_Data;


class iRDM;
class QUdpSocket;
class QTcpServer;
class QTcpSocket;
class QFile;
class iBC : public QObject
{
	Q_OBJECT

public:
	iBC(QObject *parent);
	~iBC();

protected:
	QString getIP();
	QString getMAC();
	bool	UDP_send(const MSG_PKG& msg);
	void	UDP_handle(const MSG_PKG& msg);
	void	UDP_cmd_discover(const MSG_PKG& msg);
	void	UDP_cmd_online(const MSG_PKG& msg);
	void	UDP_cmd_modbus(const MSG_PKG& msg);
	void	UDP_cmd_iot(const MSG_PKG& msg);
	void	UDP_cmd_tags(const MSG_PKG& msg);

	bool	TCP_send(const MSG_PKG& msg);

private:
	iRDM	*rdm;

	//udp socket
	QUdpSocket  *udpSocket;
	MSG_PKG		RxMsg;

	//Tcp server 
	QTcpServer	*tcpServer;
	QTcpSocket	*tcpConnection;

	//File info
	QFile		*savedfile;
	quint32		recvbytes;
	quint32		totalbytes;
	QString     filename;
	quint32		filesize;
	quint32		fileReceived;
	QByteArray  fileblock;

public slots:
	void UDP_read();
	void TCP_read();
	void TCP_connection();
	void TCP_Error(QAbstractSocket::SocketError error);
	void TCP_SocketStateChanged(QAbstractSocket::SocketState state);

signals:
	void finished();
	void receivedprogress(qint64 recevied, qint64 total);

};
