#include "EthernetCmd.h"
#include "irdm.h"
#include <QUdpSocket>
#include <QTcpSocket>
#include <QFile>
#include <QFileInfo> 

EthernetCmd::EthernetCmd(QObject *parent)
	: QObject(parent)	
{
	udpSocket = new QUdpSocket(this);
	connect(udpSocket, SIGNAL(readyRead()), this, SLOT(UDP_read()));

	tcpClient = new QTcpSocket(this);
	connect(tcpClient, SIGNAL(connected()), this, SLOT(startTransfer()));
	connect(tcpClient, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(TCP_SocketStateChanged(QAbstractSocket::SocketState)));
	connect(tcpClient, SIGNAL(bytesWritten(qint64)), this, SLOT(updateClientProgress(qint64)));
	connect(tcpClient, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(TCP_Error(QAbstractSocket::SocketError)));
	isconnected = false;
	
	
	filename	= "iTmsRdm.zip";
	payload		= 64 * 1024;			//64K payload size
	issendfile	= false;
	filewritten = 0;	
}

EthernetCmd::~EthernetCmd()
{
}

bool EthernetCmd::UDP_send(const MSG_PKG& msg)
{
	if (udpSocket && udpSocket->writeDatagram((char *)&msg.cmd_pkg, sizeof(msg.cmd_pkg), msg.rIP, msg.rPort) != sizeof(msg.cmd_pkg))
	{
		return false;
	}
	return true;
}
void EthernetCmd::UDP_read()
{
	while (udpSocket->hasPendingDatagrams())
	{
		udpSocket->readDatagram((char *)&RxMsg.cmd_pkg, sizeof(RxMsg.cmd_pkg), &RxMsg.rIP, &RxMsg.rPort);
		CMD_handle(RxMsg);
	}
}

void EthernetCmd::CMD_handle(const MSG_PKG& msg)
{
	if (msg.cmd_pkg.header.ind != UDP_IND) return;

	switch (msg.cmd_pkg.header.cmd)
	{
	case UDP_DISCOVER:
	{
		if (msg.cmd_pkg.header.len == sizeof(RDM_Paramters))
		{			
			emit newRdmReady(RxMsg);
		}
	}
	break;

	case UDP_READMODBUSSETTING:
	{
		emit ModbusParamReady(RxMsg);
	}
	break;

	case UDP_READIOTSETTING:
	{
		emit IotParaReady(RxMsg);
	}
	break;

	case UDP_READTAGSONLINE:
	{
		emit TagsOnlineReady(RxMsg);
	}
	break;

	case UDP_READTAGSSETTING:
	{
		emit TagsParaReady(RxMsg);
	}
	break;

	case UDP_READTAGSDATA:
	{
		emit TagsDataReady(RxMsg);
	}
	break;

	case UDP_FILEPARAMETER:
	{
		filewritten = 0;
		tcpClient->connectToHost(msg.rIP, RemoteTcpPort);											//tcp connect to rdm  when fileinfo udp cmd acked
	}
	break;


	default:
		break;
	}
}
void EthernetCmd::UDP_get_iotparameters(iRdm* rdm)
{
	MSG_PKG txmsg;
	txmsg.cmd_pkg.header.ind = UDP_IND;
	txmsg.cmd_pkg.header.cmd = UDP_READIOTSETTING;
	txmsg.cmd_pkg.header.len = 0;

	txmsg.rPort = RemoteUdpPort;
	txmsg.rIP = rdm->m_ip;

	UDP_send(txmsg);
}
void EthernetCmd::UDP_discoverRdm()
{
	MSG_PKG txmsg;
	txmsg.cmd_pkg.header.ind = UDP_IND;
	txmsg.cmd_pkg.header.cmd = UDP_DISCOVER;
	txmsg.cmd_pkg.header.len = 0;

	txmsg.rPort = RemoteUdpPort;
	txmsg.rIP	= QHostAddress::Broadcast;

	UDP_send(txmsg);
}
void EthernetCmd::UDP_get_modbusparameters(iRdm* rdm)
{
	MSG_PKG txmsg;
	txmsg.cmd_pkg.header.ind = UDP_IND;
	txmsg.cmd_pkg.header.cmd = UDP_READMODBUSSETTING;
	txmsg.cmd_pkg.header.len = 0;

	txmsg.rPort = RemoteUdpPort;
	txmsg.rIP = rdm->m_ip;

	UDP_send(txmsg);
}
void EthernetCmd::UDP_get_tagsonline(iRdm* rdm)
{
	MSG_PKG txmsg;
	txmsg.cmd_pkg.header.ind = UDP_IND;
	txmsg.cmd_pkg.header.cmd = UDP_READTAGSONLINE;
	txmsg.cmd_pkg.header.len = 0;

	txmsg.rPort = RemoteUdpPort;
	txmsg.rIP = rdm->m_ip;

	UDP_send(txmsg);
}
void EthernetCmd::UDP_get_tagspara(iRdm* rdm)
{
	MSG_PKG txmsg;
	txmsg.cmd_pkg.header.ind = UDP_IND;
	txmsg.cmd_pkg.header.cmd = UDP_READTAGSSETTING;
	txmsg.cmd_pkg.header.len = 0;

	txmsg.rPort = RemoteUdpPort;
	txmsg.rIP = rdm->m_ip;

	UDP_send(txmsg);
}
void EthernetCmd::UDP_get_tagsdata(iRdm* rdm)
{
	MSG_PKG txmsg;
	txmsg.cmd_pkg.header.ind = UDP_IND;
	txmsg.cmd_pkg.header.cmd = UDP_READTAGSDATA;
	txmsg.cmd_pkg.header.len = 0;

	txmsg.rPort = RemoteUdpPort;
	txmsg.rIP = rdm->m_ip;

	UDP_send(txmsg);
}
void EthernetCmd::UDP_set_tagepc(iRdm* rdm, iTag* tag)
{
	MSG_PKG txmsg;
	txmsg.cmd_pkg.header.ind = UDP_IND;
	txmsg.cmd_pkg.header.cmd = UDP_SETTAGEPC;
	txmsg.cmd_pkg.header.len = sizeof(Tag_epc);

	Tag_epc *tagEPC = (Tag_epc *)txmsg.cmd_pkg.data;
	tagEPC->uid = tag->t_uid;
	strcpy(tagEPC->epc , tag->t_epc.toLatin1());
	txmsg.rPort = RemoteUdpPort;
	txmsg.rIP = rdm->m_ip;

	UDP_send(txmsg);
}
void EthernetCmd::UDP_ipset(const QString& mac, const QString& ip)
{
	MSG_PKG txmsg;
	txmsg.cmd_pkg.header.ind = UDP_IND;
	txmsg.cmd_pkg.header.cmd = UDP_SETRDMIP;
	//to do : set ip address to Rdm
	txmsg.cmd_pkg.header.len = 0;

	txmsg.rPort = RemoteUdpPort;
	txmsg.rIP	= QHostAddress::Broadcast;

	UDP_send(txmsg);
}
void EthernetCmd::UDP_fileinfo(iRdm* rdm,QString fullpathname, FileType type)
{
	sendfile = new QFile(fullpathname);
	if (!sendfile->open(QFile::ReadOnly))
	{
		qDebug() << "Open file failed!";
		return;
	}
	QFileInfo info(fullpathname);

	filesize = info.size();
	QString filename = info.fileName();
	//first download file parameters
	File_Paramters f_para;
	memset(&f_para, 0, sizeof(f_para));
	f_para.filesize = filesize;
	f_para.filetype = type;
	if(type== XmlFile)
		strcpy(f_para.filename, "iTmsRdm.xml");
	else
		strcpy(f_para.filename, filename.toStdString().c_str());

	MSG_PKG txmsg;
	txmsg.cmd_pkg.header.ind = UDP_IND;
	txmsg.cmd_pkg.header.cmd = UDP_FILEPARAMETER;
	txmsg.cmd_pkg.header.len = sizeof(f_para);
	memcpy(txmsg.cmd_pkg.data, (char *)&f_para, sizeof(f_para));

	txmsg.rPort = RemoteUdpPort;
	txmsg.rIP	= rdm->m_ip;

	UDP_send(txmsg);
}
void EthernetCmd::TCP_Error(QAbstractSocket::SocketError error)
{
	qDebug() << tcpClient->errorString();
	tcpClient->close();
}
void EthernetCmd::TCP_SocketStateChanged(QAbstractSocket::SocketState socketState)
{
	if (socketState == QAbstractSocket::ConnectedState)
	{
		isconnected = true;
	}
	else if (socketState == QAbstractSocket::UnconnectedState)
	{
		isconnected = false;
	}
}
void EthernetCmd::startTransfer()
{
	sendblock	= sendfile->read(payload);
	leftwritten = filesize - (int)tcpClient->write(sendblock);
	sendblock.resize(0);
	issendfile = true;
}
void EthernetCmd::updateClientProgress(qint64 bytes)
{
	if (!issendfile) return;
	filewritten += bytes;
	int percent = ((float)filewritten / filesize) * 100;

	qDebug() << QString("Send %1bytes, %2%").arg(filewritten).arg(percent);
	if (leftwritten > 0)
	{
		sendblock = sendfile->read(payload);
		leftwritten -= (int)tcpClient->write(sendblock);
		sendblock.resize(0);
	}
	else
	{
		sendfile->close();
	}
}
