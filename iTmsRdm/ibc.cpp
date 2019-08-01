#include "ibc.h"
#include "irdm.h"

iBC::iBC(QObject *parent)
	: QObject(parent)
{
	rdm = (iRDM *)parent;

	//udp socket
	udpSocket = new QUdpSocket(this);
	udpSocket->bind(UDP_PORT, QUdpSocket::ShareAddress);
	connect(udpSocket, SIGNAL(readyRead()), this, SLOT(UDP_read()));

	//TCP Server
	tcpServer = new QTcpServer(this);
	connect(tcpServer, SIGNAL(newConnection()), this, SLOT(TCP_connection()));
	if (!tcpServer->listen(QHostAddress::LocalHost, TCP_PORT))
	{
		qDebug() << "Tcp Server Listening Error!";
	}

	fileReceived = 0;
	filesize = 0;
	savedfile = NULL;
}

iBC::~iBC()
{
}

QString iBC::getIP()
{
	QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
	foreach(QHostAddress address, addresses)
	{
		if (address.protocol() == QAbstractSocket::IPv4Protocol)
			return address.toString();
	}
	return 0;
}

QString iBC::getMAC()
{

	QList<QNetworkInterface> nets = QNetworkInterface::allInterfaces();
	foreach(QNetworkInterface net, nets)
	{
		if (net.flags().testFlag(QNetworkInterface::IsUp)
			&& net.flags().testFlag(QNetworkInterface::IsRunning)
			&& !net.flags().testFlag(QNetworkInterface::IsLoopBack))
			return net.hardwareAddress();
	}
	return QString("");
}


void iBC::UDP_read()
{
	while (udpSocket->hasPendingDatagrams())
	{
		udpSocket->readDatagram((char *)&RxMsg.cmd_pkg, sizeof(RxMsg.cmd_pkg), &RxMsg.rIP, &RxMsg.rPort);
		UDP_handle(RxMsg);
	}
}
bool iBC::UDP_send(const MSG_PKG& msg)
{
	if (udpSocket && udpSocket->writeDatagram((char *)&msg.cmd_pkg, sizeof(msg.cmd_pkg), msg.rIP, msg.rPort) != sizeof(msg.cmd_pkg))
	{
		return false;
	}
	return true;
}
void iBC::UDP_handle(const MSG_PKG& msg)
{
	if (msg.cmd_pkg.header.ind != UDP_IND) return;

	switch (msg.cmd_pkg.header.cmd)
	{
	case UDP_DISCOVER:
	{
		MSG_PKG txMsg;
		txMsg.cmd_pkg.header.ind = UDP_IND;
		txMsg.cmd_pkg.header.cmd = UDP_DISCOVER;

		//Test datas
		RDM_Paramters rdm_p;
		memset(&rdm_p, 0, sizeof(rdm_p));
		strcpy(rdm_p.RdmIp,		getIP().toLatin1());
		strcpy(rdm_p.RdmName,	rdm->RDM_name.toLatin1());
		strcpy(rdm_p.RdmMAC,	getMAC().toLatin1());

		//todo: send back local ip to remote
		txMsg.cmd_pkg.header.len = sizeof(rdm_p);
		memcpy(txMsg.cmd_pkg.data, &rdm_p, sizeof(rdm_p));

		txMsg.rIP	= msg.rIP;
		txMsg.rPort = msg.rPort;
		UDP_send(txMsg);
	}
	break;

	case UDP_FILEPARAMETER:
	{
		if (msg.cmd_pkg.header.len == sizeof(File_Paramters))
		{
			File_Paramters *file_para = (File_Paramters *)msg.cmd_pkg.data;
			if (file_para)
			{
				filesize = file_para->filesize;
				filename = file_para->filename;

				MSG_PKG txMsg;
				txMsg.cmd_pkg.header.ind = UDP_IND;
				txMsg.cmd_pkg.header.cmd = UDP_FILEPARAMETER;

				txMsg.cmd_pkg.header.len = 0;
				TCP_send(txMsg);

				savedfile = new QFile(QCoreApplication::applicationDirPath() + "/" + filename);
				if (!savedfile->open(QFile::WriteOnly))
				{
					qDebug() << "New file failed!";
					return;
				}
			}
		}
	}
	break;
	}
}

bool iBC::TCP_send(const MSG_PKG& msg)
{
	if (tcpConnection->state() != QAbstractSocket::ConnectedState) return false;

	QByteArray outBlock;
	outBlock.append((char*)&msg.cmd_pkg, sizeof(msg.cmd_pkg));

	if (tcpConnection->write(outBlock) < 0) return false;
	else 	return true;
}

void iBC::TCP_connection()
{
	tcpConnection = tcpServer->nextPendingConnection();
	connect(tcpConnection, SIGNAL(readyRead()), this, SLOT(TCP_read()));
	connect(tcpConnection, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(Tcp_SocketStateChanged(QAbstractSocket::SocketState)));
	connect(tcpConnection, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(Tcp_Error(QAbstractSocket::SocketError)));
}

void iBC::Tcp_SocketStateChanged(QAbstractSocket::SocketState state)
{
	if (state == QAbstractSocket::UnconnectedState)
	{
		fileReceived = 0;
		filesize = 0;
		if (savedfile)
			savedfile->close();
	}
}

void iBC::TCP_read()
{
	if ((filesize > 0) && (!filename.isEmpty()))
	{
		//start save file			
		fileReceived += tcpConnection->bytesAvailable();
		fileblock = tcpConnection->readAll();
		savedfile->write(fileblock);
		fileblock.resize(0);
		if (fileReceived == filesize)
		{
			//saved ok 
			savedfile->close();
			fileReceived = 0;
		}
	}
}

void iBC::Tcp_Error(QAbstractSocket::SocketError error)
{
	qDebug() << tcpConnection->errorString();
	tcpConnection->close();
	if (savedfile)
		savedfile->close();
}
