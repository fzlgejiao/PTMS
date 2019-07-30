#include "EthernetCmd.h"
#include <QUdpSocket>
#include <QTcpSocket>
#include <QFile>

EthernetCmd::EthernetCmd(QObject *parent)
	: QObject(parent)	
{
	udpSocket = new QUdpSocket(this);
	connect(udpSocket, SIGNAL(readyRead()), this, SLOT(UDP_read()));

	tcpClient = new QTcpSocket(this);
	//connect(tcpClient, SIGNAL(connected()), this, SLOT(onConnectedServer()));
	connect(tcpClient, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onConnectionstatechanged(QAbstractSocket::SocketState)));
	connect(tcpClient, SIGNAL(readyRead()), this, SLOT(onTcp_readyRead()));
	connect(tcpClient, SIGNAL(bytesWritten(qint64)), this, SLOT(updateClientProgress(qint64)));
	connect(tcpClient, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onTcpError(QAbstractSocket::SocketError)));
	isconnected = false;

	tcpClient->connectToHost(QHostAddress::LocalHost, RemoteTcpPort);
	connect(this, SIGNAL(fileSendready()), this, SLOT(startTransfer()));
	filename = "iTmsRdm.zip";
	payload = 64 * 1024;			//64K payload size
	issendfile = false;
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
void EthernetCmd::onTcp_readyRead()
{
	QByteArray data = tcpClient->readAll();
	memcpy(&RxMsg.cmd_pkg, data, data.length());
	CMD_handle(RxMsg);
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
			emit newRdmready(RxMsg);
		}
	}
	break;

	case UDP_FILEPARAMETER:
	{
		emit fileSendready();
	}
	break;


	default:
		break;
	}
}
void EthernetCmd::discoverRdm()
{
	MSG_PKG txmsg;
	txmsg.cmd_pkg.header.ind = UDP_IND;
	txmsg.cmd_pkg.header.cmd = UDP_DISCOVER;
	txmsg.cmd_pkg.header.len = 0;

	txmsg.rPort = RemoteUdpPort;
	txmsg.rIP = QHostAddress::Broadcast;

	UDP_send(txmsg);
}
void EthernetCmd::ipset(QString ipaddress)
{
	MSG_PKG txmsg;
	txmsg.cmd_pkg.header.ind = UDP_IND;
	txmsg.cmd_pkg.header.cmd = UDP_SETIP;
	//to do : set ip address to Rdm
	txmsg.cmd_pkg.header.len = 0;

	txmsg.rPort = RemoteUdpPort;
	txmsg.rIP = QHostAddress::Broadcast;

	UDP_send(txmsg);
}
void EthernetCmd::downloadfile(QString fullpathname)
{
	sendfile = new QFile(fullpathname);
	if (!sendfile->open(QFile::ReadOnly))
	{
		qDebug() << "Open file failed!";
		return;
	}
	filesize = sendfile->size();
	//first download file parameters
	File_Paramters f_para;
	int p_size = sizeof(f_para);

	f_para.filesize = filesize;
	memset(f_para.filename, 0, sizeof(f_para.filename));
	strcpy(f_para.filename, filename.toStdString().c_str());

	MSG_PKG txmsg;
	txmsg.cmd_pkg.header.ind = UDP_IND;
	txmsg.cmd_pkg.header.cmd = UDP_FILEPARAMETER;
	txmsg.cmd_pkg.header.len = p_size;
	memcpy(txmsg.cmd_pkg.data, (char *)&f_para, p_size);

	QByteArray sendbytes;
	sendbytes.append((char*)&txmsg.cmd_pkg, sizeof(txmsg.cmd_pkg));

	tcpClient->write(sendbytes);
}
void EthernetCmd::onTcpError(QAbstractSocket::SocketError error)
{
	qDebug() << tcpClient->errorString();
	tcpClient->close();
}
void EthernetCmd::onConnectedServer()
{
	isconnected = true;
}
void EthernetCmd::onConnectionstatechanged(QAbstractSocket::SocketState socketState)
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
	sendblock = sendfile->read(payload);
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
