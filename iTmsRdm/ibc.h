#pragma once

#include <QObject>
#include <QtNetwork/QUdpSocket>

#define UDP_IND		0xAA55

#define UDP_PORT	2900
#define TCP_PORT	2901

#define	TAG_NUM		12

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
	UDP_READTAGSSETTING		= 5,
	UDP_READTAGSONLINE		= 6,
	UDP_READTAGSDATA		= 7,
	UDP_SETTAGEPC			= 8,
	UDP_SETRDMIP			= 9,
	UDP_FILEPARAMETER = 0x10
}UDP_CMD;


typedef struct {
	quint32	fileSize;
	char	fileName[16];
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
		char	note[32];
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
		quint16	reserved;
		char	name[16];
		char	note[32];
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
	void	UDP_cmd_tags_para(const MSG_PKG& msg);
	void	UDP_cmd_tags_online(const MSG_PKG& msg);
	void	UDP_cmd_tags_data(const MSG_PKG& msg);
	void	UDP_cmd_tag_epc(const MSG_PKG& msg);
	void	UDP_cmd_rdm_ip(const MSG_PKG& msg);
	void	UDP_cmd_file(const MSG_PKG& msg);

	void	TCP_start();
private:
	iRDM	*rdm;

	//udp socket
	QUdpSocket  *udpSocket;
	MSG_PKG		RxMsg;

	//Tcp server 
	QTcpServer	*tcpServer;
	QTcpSocket	*tcpConnection;

	//File info
	QFile		*savedFile;
	quint32		recvBytes;
	quint32		totalBytes;
	quint32		fileNameSize;
	QString     fileName;
	QByteArray  fileBlock;

public slots:
	void UDP_read();
	void TCP_read();
	void TCP_connection();
	void TCP_Error(QAbstractSocket::SocketError error);
	void TCP_SocketStateChanged(QAbstractSocket::SocketState state);
	void OnFileDone(bool ok);

signals:
	void fileDone(bool ok);

};
