#include "ibc.h"
#include "irdm.h"
#include "ireader.h"

#ifdef __linux__
#include <unistd.h>
#endif

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

	TcpStartListen();

	recvBytes = 0;
	totalBytes = 0;
	savedFile = NULL;
	tcpConnection = NULL;

	connect(this, SIGNAL(fileDone(bool)), this, SLOT(OnFileDone(bool)));

	connect(this, SIGNAL(reloadXml()), rdm, SLOT(RDM_init()));
	connect(this, SIGNAL(upgrade(QString )), this, SLOT(OnUpgradeRdm(QString)));

	connect(this, SIGNAL(ipchanged(bool)), this, SLOT(TcpStartListen()));
}

iBC::~iBC()
{
	if(tcpConnection && tcpConnection->isOpen())
		tcpConnection->close();
	tcpServer->close();
	OnFileDone(false);
}

void iBC::TcpStartListen()
{
	if (tcpServer->isListening())
	{
		tcpServer->close();
	}
	if (!tcpServer->listen(QHostAddress(getIP()), TCP_PORT))
	{
		qDebug() << "Tcp Server Listening Error!";
	}
	else
	{
		qDebug() << "Rdm:" << "Tcp listenning in " << getIP();
	}
}

QString iBC::getIP()
{
	QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
	for(QHostAddress address: addresses)
	{
		if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress::LocalHost)
			return address.toString();
	}
	return 0;
}

QString iBC::getMAC()
{

	QList<QNetworkInterface> nets = QNetworkInterface::allInterfaces();
	for(QNetworkInterface net: nets)
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
			UDP_cmd_discover(msg);
		}
		break;

		case UDP_ONLINE:
		{
			UDP_cmd_online(msg);
		}
		break;

		case UDP_READMODBUSSETTING:
		{
			UDP_cmd_modbus(msg);
		}
		break;

		case UDP_READIOTSETTING:
		{
			UDP_cmd_iot(msg);
		}
		break;

		case UDP_READTAGSDATA:
		{
			UDP_cmd_tags_data(msg);
		}
		break;

		case UDP_READTAGSONLINE:
		{
			UDP_cmd_tags_online(msg);
		}
		break;

		case UDP_READTAGSSETTING:
		{
			UDP_cmd_tags_para(msg);
		}
		break;

		case UDP_SETTAGEPC:
		{
			UDP_cmd_tag_epc(msg);
		}
		break;

		case UDP_FILEPARAMETER:
		{
			UDP_cmd_file(msg);
		}
		break;

		case UDP_SETRDMIP:
		{
			UDP_cmd_rdm_ip(msg);
		}
		break;
	}
}

void iBC::TCP_connection()
{
	tcpConnection = tcpServer->nextPendingConnection();
	connect(tcpConnection, SIGNAL(readyRead()), this, SLOT(TCP_read()));
	connect(tcpConnection, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(TCP_SocketStateChanged(QAbstractSocket::SocketState)));
	connect(tcpConnection, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(TCP_Error(QAbstractSocket::SocketError)));
}

void iBC::TCP_SocketStateChanged(QAbstractSocket::SocketState state)
{
	if (state == QAbstractSocket::UnconnectedState)
	{
		emit fileDone(false);
	}
}

void iBC::TCP_Error(QAbstractSocket::SocketError error)
{
	qDebug() << tcpConnection->errorString();
	tcpConnection->close();

	emit fileDone(false);
}

void iBC::TCP_start()
{
	OnFileDone(false);
}

void iBC::TCP_read()
{
/*
	QDataStream in(tcpConnection);
	in.setVersion(QDataStream::Qt_5_12);

	if (recvBytes <= sizeof(quint32) * 2) {
		if ((tcpConnection->bytesAvailable() >= sizeof(quint32) * 2)
			&& (fileNameSize == 0)) {

			in >> totalBytes >> fileNameSize;
			recvBytes += sizeof(quint32) * 2;
		}
		if ((tcpConnection->bytesAvailable() >= fileNameSize)
			&& (fileNameSize != 0)) {

			in >> fileName;
			qDebug() << tr("接收文件%1 ...").arg(fileName);

			recvBytes += fileNameSize;
			savedFile = new QFile(fileName);
			if (!savedFile->open(QFile::WriteOnly)) {
				qDebug() << "server: open file error!";
				return;
			}
		}
		else {
			return;
		}
	}
*/

	if (recvBytes == 0)
	{
		QString savepath = QString("%1/").arg(QCoreApplication::applicationDirPath());
		savedFile = new QFile(savepath + fileName);
		if (!savedFile->open(QFile::WriteOnly | QFile::Truncate)) {
			qDebug() << "server: open file error!";
			return;
		}
	}
	if (recvBytes < totalBytes) {
		recvBytes += tcpConnection->bytesAvailable();
		fileBlock = tcpConnection->readAll();
		savedFile->write(fileBlock);
		fileBlock.resize(0);
		qDebug() << tr("Write %1bytes to %2!").arg(recvBytes).arg(fileName);
	}

	if (recvBytes == totalBytes) {
		tcpConnection->close();
	
		emit fileDone(true);

		qDebug() << tr("Receive %1 ok!").arg(fileName);
	}
}

void iBC::OnFileDone(bool ok)
{
	if (savedFile)
	{
		if (savedFile->isOpen())
			savedFile->close();
		delete savedFile;
		savedFile = NULL;
	}
	recvBytes = 0;
	if (ok)
	{
		totalBytes = 0;
		switch (filetype)
		{
		case XmlFile:
			emit reloadXml();
			break;

		case TarFile:
			emit upgrade(fileName);
			break;		
		}		
	}
}

void iBC::UDP_cmd_discover(const MSG_PKG& msg)
{//cmd=0x01, ack discover for rdms
	//Test datas
	RDM_Paramters rdm_p;
	memset(&rdm_p, 0, sizeof(rdm_p));
	strcpy(rdm_p.RdmIp,		getIP().toLatin1());
	strcpy(rdm_p.RdmName,	rdm->RDM_name.toLocal8Bit());
	strcpy(rdm_p.RdmMAC,	getMAC().toLatin1());
	strcpy(rdm_p.RdmVersion, qApp->applicationVersion().toLatin1());
	strcpy(rdm_p.RdmNote,	rdm->RDM_note.toLocal8Bit());
	strcpy(rdm_p.RdmComName, rdm->RDM_comname.toLatin1());

	//todo: send back local ip to remote
	MSG_PKG txMsg;
	memset(&txMsg.cmd_pkg, 0, sizeof(txMsg.cmd_pkg));
	txMsg.cmd_pkg.header.ind = UDP_IND;
	txMsg.cmd_pkg.header.cmd = UDP_DISCOVER;	
	txMsg.cmd_pkg.header.len = sizeof(rdm_p);
	memcpy(txMsg.cmd_pkg.data, &rdm_p, sizeof(rdm_p));

	//txMsg.rIP = msg.rIP;
	txMsg.rIP = QHostAddress::Broadcast;
	txMsg.rPort = msg.rPort;
	UDP_send(txMsg);	
}

void iBC::UDP_cmd_online(const MSG_PKG& msg)
{//cmd=0x02, ack online check
	MSG_PKG txMsg;
	memset(&txMsg.cmd_pkg, 0, sizeof(txMsg.cmd_pkg));
	txMsg.cmd_pkg.header.ind = UDP_IND;
	txMsg.cmd_pkg.header.cmd = UDP_ONLINE;
	txMsg.cmd_pkg.header.len = 16;

	memcpy(txMsg.cmd_pkg.data, msg.cmd_pkg.data, msg.cmd_pkg.header.len);

	txMsg.rIP = msg.rIP;
	txMsg.rPort = msg.rPort;
	UDP_send(txMsg);
}

void iBC::UDP_cmd_modbus(const MSG_PKG& msg)
{//cmd=0x03, ack modbus settings
	MSG_PKG txMsg;
	memset(&txMsg.cmd_pkg, 0, sizeof(txMsg.cmd_pkg));
	txMsg.cmd_pkg.header.ind = UDP_IND;
	txMsg.cmd_pkg.header.cmd = UDP_READMODBUSSETTING;

	MODBUS_Paramters modbus;
	memset(&modbus, 0, sizeof(modbus));
	modbus.type			= rdm->modbustype == "RTU" ? 0 : 1;
	strcpy(modbus.rtu_comname, rdm->rtucomname.toLatin1());
	modbus.rtu_baudrate	= rdm->rtubaudrate;
	modbus.rtu_address	= rdm->rtuslaveaddress;
	modbus.rtu_parity	= rdm->rtuparity;
	modbus.tcp_port		= rdm->TcpPort;

	txMsg.cmd_pkg.header.len = sizeof(modbus);
	memcpy(txMsg.cmd_pkg.data, &modbus, sizeof(modbus));

	txMsg.rIP = msg.rIP;
	txMsg.rPort = msg.rPort;
	UDP_send(txMsg);
}

void iBC::UDP_cmd_iot(const MSG_PKG& msg)
{//cmd=0x04,ack IOT settings
	MSG_PKG txMsg;
	txMsg.cmd_pkg.header.ind = UDP_IND;
	txMsg.cmd_pkg.header.cmd = UDP_READIOTSETTING;

	IOT_Paramters iot;
	memset(&iot, 0, sizeof(iot));
	strcpy(iot.devicename, rdm->devicename.toLatin1());
	strcpy(iot.productkey, rdm->productkey.toLatin1());
	strcpy(iot.devicesecret, rdm->devicesecret.toLatin1());
	strcpy(iot.regionid, rdm->regionid.toLatin1());

	txMsg.cmd_pkg.header.len = sizeof(iot);
	memcpy(txMsg.cmd_pkg.data, &iot, sizeof(iot));

	txMsg.rIP = msg.rIP;
	txMsg.rPort = msg.rPort;
	UDP_send(txMsg);

}
void iBC::UDP_cmd_tags_para(const MSG_PKG& msg)
{//cmd=0x05,ack tags settings
	MSG_PKG txMsg;
	memset(&txMsg.cmd_pkg, 0, sizeof(txMsg.cmd_pkg));
	txMsg.cmd_pkg.header.ind = UDP_IND;
	txMsg.cmd_pkg.header.cmd = UDP_READTAGSSETTING;
	txMsg.cmd_pkg.header.len = sizeof(Tags_Parameters);

	Tags_Parameters *tagsdata = (Tags_Parameters *)txMsg.cmd_pkg.data;
	tagsdata->Header.tagcount = rdm->taglist.count();

	int idx = 0;
	for (iTag *tag : rdm->taglist)
	{
		tagsdata->Tags[idx].uid = tag->T_uid;
		tagsdata->Tags[idx].sid = tag->T_sid;
		tagsdata->Tags[idx].upperlimit = tag->T_uplimit;
		strcpy(tagsdata->Tags[idx].name, tag->T_epc.toLocal8Bit());
		strcpy(tagsdata->Tags[idx].note, tag->T_note.toLocal8Bit());
		idx++;
	}

	txMsg.rIP = msg.rIP;
	txMsg.rPort = msg.rPort;
	UDP_send(txMsg);
}
void iBC::UDP_cmd_tags_online(const MSG_PKG& msg)
{//cmd=0x06,ack tags online
	MSG_PKG txMsg;
	memset(&txMsg.cmd_pkg, 0, sizeof(txMsg.cmd_pkg));
	txMsg.cmd_pkg.header.ind = UDP_IND;
	txMsg.cmd_pkg.header.cmd = UDP_READTAGSONLINE;
	txMsg.cmd_pkg.header.len = sizeof(Tags_Online);

	Tags_Online *tagsdata = (Tags_Online *)txMsg.cmd_pkg.data;

	qDebug() << "Message online tags :";

	int idx = 0;
	QMapIterator<quint64, QByteArray> i(rdm->tagOnline);
	while (i.hasNext()) {
		i.next();
		tagsdata->Tags[idx].uid = i.key();
		memcpy(tagsdata->Tags[idx].name, i.value().data(),i.value().count());
		idx++;
		qDebug() << "uid = " << i.key() << " epc = " << i.value();
	}
	tagsdata->Header.tagcount = idx;																//online tags count

	txMsg.rIP = msg.rIP;
	txMsg.rPort = msg.rPort;
	UDP_send(txMsg);
}

void iBC::UDP_cmd_tags_data(const MSG_PKG& msg)
{//cmd=0x07,ack tags data
	MSG_PKG txMsg;
	memset(&txMsg.cmd_pkg, 0, sizeof(txMsg.cmd_pkg));
	txMsg.cmd_pkg.header.ind = UDP_IND;
	txMsg.cmd_pkg.header.cmd = UDP_READTAGSDATA;
	txMsg.cmd_pkg.header.len = sizeof(Tags_Data);

	Tags_Data *tagsdata = (Tags_Data *)txMsg.cmd_pkg.data;
	tagsdata->Header.tagcount = rdm->taglist.count();

	int idx = 0;
	for (iTag *tag : rdm->taglist)
	{
		tagsdata->Tags[idx].uid			= tag->T_uid;
		tagsdata->Tags[idx].sid			= tag->T_sid;
		tagsdata->Tags[idx].alarm		= tag->isAlarm() ? 1 : 0;
		tagsdata->Tags[idx].rssi		= tag->T_rssi;
		tagsdata->Tags[idx].oc_rssi		= tag->T_OC_rssi;
		tagsdata->Tags[idx].temperature = tag->T_temp*10;
		tagsdata->Tags[idx].online		= tag->isonline()?1 : 0;
		strcpy(tagsdata->Tags[idx].name, tag->T_epc.toLocal8Bit());
		strcpy(tagsdata->Tags[idx].note, tag->T_note.toLocal8Bit());
		idx++;
	}

	txMsg.rIP	= msg.rIP;
	txMsg.rPort = msg.rPort;
	UDP_send(txMsg);
}

void iBC::UDP_cmd_tag_epc(const MSG_PKG& msg)
{//cmd=0x08,ack write tag epc
	//write tag epc
	Tag_epc *rtagepc = (Tag_epc *)msg.cmd_pkg.data;

	QByteArray epc_old = rdm->tagOnline.value(rtagepc->uid);

	memcpy(&rdm->tagWrite, rtagepc, sizeof(Tag_epc));
/*
	bool ret = rdm->reader->wirteEpc(epc_old, rtagepc->epc);

	iTag* tag = rdm->Tag_get(QString(epc_old));
	if (tag)
	{
		if (ret)
			tag->T_epc = rtagepc->epc;
		else
			tag->T_epc = epc_old;
	}


	//ack back
	MSG_PKG txMsg;
	memset(&txMsg.cmd_pkg, 0, sizeof(txMsg.cmd_pkg));
	txMsg.cmd_pkg.header.ind = UDP_IND;
	txMsg.cmd_pkg.header.cmd = UDP_SETTAGEPC;
	txMsg.cmd_pkg.header.len = sizeof(Tag_epc);

	Tag_epc *tagpec = (Tag_epc *)txMsg.cmd_pkg.data;
	tagpec->uid = rtagepc->uid;
	if(ret)
		strcpy(tagpec->epc , rtagepc->epc);
	else
		memcpy(tagpec->epc, epc_old.data(), epc_old.count());

	txMsg.rIP = msg.rIP;
	txMsg.rPort = msg.rPort;
	UDP_send(txMsg);
*/
}

void iBC::UDP_cmd_rdm_ip(const MSG_PKG& msg)
{//cmd=0x09,ack set rdm ip
	IpSetFormat *ipset = (IpSetFormat *)msg.cmd_pkg.data;

	QString localmac = getMAC();
	QString mac = ipset->mac;
	QString ipadress = ipset->ip;

	if(QString::compare(localmac,mac)==0)		//if my own mac
	{
#ifdef __linux__
		QString setipcmd = QString("ifconfig eth0 %1 netmask 255.255.255.0").arg(ipadress);
		system(setipcmd.toStdString().c_str());

		QString setfilecmd = QString("sed -i '7s#.*#IPv4 = %1/255.255.255.0#g' /var/lib/connman/wired.config").arg(ipadress);
		system(setfilecmd.toStdString().c_str());
		system("sync");

#endif
		emit ipchanged(true);
	}
}

void iBC::UDP_cmd_file(const MSG_PKG& msg)
{//cmd=0x10,ack download file info
	if (msg.cmd_pkg.header.len == sizeof(File_Paramters))
	{
		File_Paramters *file_para = (File_Paramters *)msg.cmd_pkg.data;
		if (file_para)
		{
			totalBytes	= file_para->fileSize;
			fileName	= file_para->fileName;
			filetype = (FileType)file_para->filetype;

			MSG_PKG txMsg;
			txMsg.cmd_pkg.header.ind = UDP_IND;
			txMsg.cmd_pkg.header.cmd = UDP_FILEPARAMETER;

			txMsg.cmd_pkg.header.len = 0;
			txMsg.rIP	= msg.rIP;
			txMsg.rPort = msg.rPort;
			UDP_send(txMsg);

			TCP_start();																			//start a new file
		}
	}
}
void iBC::OnUpgradeRdm(QString tarfilename)
{
#ifdef __linux__
	QString directory = QCoreApplication::applicationDirPath() + "/";
	QString appname = "iTmsRdm";
	QString backupcmd = QString("mv %1%2 %3%4.bk").arg(directory).arg(appname).arg(directory).arg(appname);
	system(backupcmd.toStdString().c_str());

	QString tarcmd = QString("tar -xzvf %1 -C %2").arg(directory + tarfilename).arg(directory);
	system(tarcmd.toStdString().c_str());
	
	system("sync");

	qDebug() << "Restart the rdm service";
	system("systemctl restart rdm");
#else
	qDebug() << "OnUpgradeRdm start!";
#endif
}
