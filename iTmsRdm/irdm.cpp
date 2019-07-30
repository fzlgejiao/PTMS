#include "irdm.h"
#include "ireader.h"
#include "itag.h"
#include <QDebug> 
#include <QtNetwork/QUdpSocket>

iRDM *iRDM::_RDM = 0;
iRDM::iRDM(QObject *parent)
	: QObject(parent)	
{
#ifdef WIN32
	comName = "tmr:///com3";
#else
	comName = "tmr:///dev/ttyUSB0";
#endif
	QString xml = QCoreApplication::applicationDirPath() + "/iTmsRdm.xml";
	Cfg_load(xml);

	reader		= new iReader(comName, this);
	iotdevice	= new iDevice(this);
	modbus = new CModbus(this);

	iotdevice->IOT_init();
	if (modbustype == "RTU")
		modbus->initModbusRTUSlave(rtuserial, rtuslaveaddress, rtubaudrate, (QSerialPort::Parity)rtuparity);
	else if (modbustype == "TCP")
		modbus->initModbusTCP(RDM_ip, TcpPort);

	RDM_available = false;

	//set tag flags for send the tag info to system
	for (iTag *tag : taglist)
	{
		tag->T_data_flag = Tag_UID | Tag_EPC | Tag_Upperlimit | Tag_Switch | Tag_Rssi | Tag_Temperature;
	}

	RDM_ticks = RDM_TICKS;
	Tmr_start();

	//udp socket
	udpSocket = new QUdpSocket(this);
	udpSocket->bind(UdpLocalPort, QUdpSocket::ShareAddress);
	connect(udpSocket, SIGNAL(readyRead()), this, SLOT(UDP_read()));
	//TCP Server
	connect(&m_tcpServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
	if (!m_tcpServer.listen(QHostAddress::LocalHost, TcpServerPort))
	{
		qDebug() << "Tcp Server Listening Error!";
	}	

	fileReceived = 0;
	filesize = 0;
	savedfile = NULL;
}

iRDM::~iRDM()
{
}
bool iRDM::Cfg_load(const QString& xml)
{
	//parse xml 
	QFile file(xml);
	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		//std::cerr << "Error: Cannot read file " << qPrintable(xml)
		//	<< ": " << qPrintable(file.errorString())
		//	<< std::endl;
		return false;
	}
	//clear old tag list
	qDeleteAll(taglist);
	taglist.clear();

	QXmlStreamReader xmlReader;
	xmlReader.setDevice(&file);

	xmlReader.readNext();
	while (!xmlReader.atEnd())
	{
		if (xmlReader.isStartElement())
		{
			if (xmlReader.name() == "rdm")
			{
				//get data file name
				RDM_mac = xmlReader.attributes().value("mac").toString();
				RDM_name = xmlReader.attributes().value("name").toString();
				RDM_ip = xmlReader.attributes().value("ip").toString();
				Cfg_readrdm(xmlReader);																//read rdm
			}
			else
			{
				xmlReader.raiseError(QObject::tr("Not a rdm config file"));
			}
		}
		else
		{
			xmlReader.readNext();
		}
	}

	file.close();
	if (xmlReader.hasError()) {
		//std::cerr << "Error: Failed to parse file "
		//	<< qPrintable(fileName) << ": "
		//	<< qPrintable(xmlReader.errorString()) << std::endl;
		return false;
	}
	else if (file.error() != QFile::NoError) {
		//std::cerr << "Error: Cannot read file " << qPrintable(fileName)
		//	<< ": " << qPrintable(file.errorString())
		//	<< std::endl;
		return false;
	}
	return true;
}
void iRDM::Cfg_readrdm(QXmlStreamReader& xmlReader)
{
	xmlReader.readNext();
	while (!xmlReader.atEnd()) {
		if (xmlReader.isEndElement()) {

			xmlReader.readNext();
			break;
		}

		if (xmlReader.isStartElement()) {
			if (xmlReader.name() == "cfg")
			{
				Cfg_readcfg(xmlReader);
			}
			else if (xmlReader.name() == "tags")
			{
				Cfg_readtags(xmlReader);
			}
			else
			{
				Cfg_skipUnknownElement(xmlReader);
			}
		}
		else {
			xmlReader.readNext();
		}
	}
}
void iRDM::Cfg_readcfg(QXmlStreamReader& xmlReader)
{
	xmlReader.readNext();
	while (!xmlReader.atEnd()) {
		if (xmlReader.isEndElement()) {
			xmlReader.readNext();
			break;
		}

		if (xmlReader.isStartElement()) {
			if (xmlReader.name() == "com") {
				comName = xmlReader.readElementText();											//read com port

				if (xmlReader.isEndElement())
					xmlReader.readNext();
			}
			else if (xmlReader.name() == "iot")
			{
				productkey = xmlReader.attributes().value("productkey").toString();
				devicename = xmlReader.attributes().value("devicename").toString();
				devicesecret = xmlReader.attributes().value("devicesecret").toString();
				regionid = xmlReader.attributes().value("regionid").toString();
				QString iot = xmlReader.readElementText();
				if (xmlReader.isEndElement())
					xmlReader.readNext();
			}
			else if (xmlReader.name() == "modbus")
			{
				modbustype = xmlReader.attributes().value("type").toString();
				rtuserial = xmlReader.attributes().value("comname").toString();
				rtuslaveaddress = xmlReader.attributes().value("slaveaddress").toInt();
				rtubaudrate = xmlReader.attributes().value("baudrate").toInt();
				rtuparity = xmlReader.attributes().value("parity").toInt();
				TcpPort = xmlReader.attributes().value("tcpport").toInt();
				
				xmlReader.readElementText();
				if (xmlReader.isEndElement())
					xmlReader.readNext();
			}
			else
			{
				Cfg_skipUnknownElement(xmlReader);
			}
		}
		else {
			xmlReader.readNext();
		}
	}
}
void iRDM::Cfg_readtags(QXmlStreamReader& xmlReader)
{
	//reset tag list
	qDeleteAll(taglist);
	taglist.clear();

	qDebug() << "Cfg_readtags:" << endl;

	xmlReader.readNext();
	while (!xmlReader.atEnd()) {
		if (xmlReader.isEndElement()) {
			xmlReader.readNext();
			break;
		}

		if (xmlReader.isStartElement()) {
			if (xmlReader.name() == "tag")
			{
				int sid;
				quint64 uid;
				QString epc;
				int enable;
				int max = TAG_T_MAX;
				sid = xmlReader.attributes().value("sid").toString().toInt();
				uid = xmlReader.attributes().value("uid").toString().toULongLong();
				epc = xmlReader.attributes().value("epc").toString();
				if(xmlReader.attributes().value("max").isEmpty() == false)
					max = xmlReader.attributes().value("max").toString().toInt();

				//todo: load tag info
				iTag *tag = Tag_add(sid, uid, epc);
				if (tag)
					tag->T_uplimit = max;
				qDebug() << "tag : sid = " << sid 
						<< " uid = " << uid 
					    << " epc = " << epc << endl;

				QString iot = xmlReader.readElementText();
				if (xmlReader.isEndElement())
					xmlReader.readNext();
			}
			else
			{
				Cfg_skipUnknownElement(xmlReader);
			}
		}
		else {
			xmlReader.readNext();
		}
	}
}
void iRDM::Cfg_skipUnknownElement(QXmlStreamReader& xmlReader)
{
	xmlReader.readNext();
	while (!xmlReader.atEnd()) {
		if (xmlReader.isEndElement()) {
			xmlReader.readNext();
			break;
		}

		if (xmlReader.isStartElement()) {
			Cfg_skipUnknownElement(xmlReader);
		}
		else {
			xmlReader.readNext();
		}
	}
}
iTag* iRDM::Tag_add(int sid, quint64 uid, const QString& epc)
{
	iTag *tag = new iTag(sid,uid,epc, this);
	if (tag)
	{
		taglist.insert(tag->T_uid, tag);
	}
	return tag;
}
iTag* iRDM::Tag_getbysid(int sid)
{
	for (iTag *tag : taglist)
	{
		if (tag->T_sid == sid)
			return tag;
	}
	return NULL;
}
void iRDM::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == timerId)
	{
		//read tags
		reader->readtag();

		//upload rdm data
		iotdevice->IOT_tick();
		iotdevice->PUB_rdm_event();

		//upload tag data																				
		for (iTag* tag : taglist)
		{
			if (tag->T_enable == false)
				continue;

			tag->T_data_flag |= Tag_Online;
			if (tag->isonline())
				tag->T_data_flag |= Tag_Temperature|Tag_Rssi|Tag_Alarm;
			

			if (tag->T_ticks)
			{
				tag->T_ticks--;
				if (tag->T_ticks == 0)
				{
					qDebug() << "tag : sid = " << tag->T_sid 
						<< " uid = " << tag->T_uid 
						<< " Alarm : Offline" << endl;
					tag->T_alarm_offline = true;													//offline

				}
			}
			if (tag->T_ticks == 0)
			{
				qDebug() << "tag : sid = " << tag->T_sid
					<< " uid = " << tag->T_uid
					<< " Offline" << endl;
			}

			iotdevice->PUB_tag_data(tag);
			iotdevice->PUB_tag_event(tag);
			modbus->updateRdmRegisters(tag);
		}
	}
	if (event->timerId() == timer_datetime)  //update modbus datetime registers
	{
		modbus->updatesystime(QDateTime::currentDateTime());
	}
}

void iRDM::UDP_read()
{
	while (udpSocket->hasPendingDatagrams())
	{
		udpSocket->readDatagram((char *)&RxMsg.cmd_pkg, sizeof(RxMsg.cmd_pkg), &RxMsg.rIP,&RxMsg.rPort);
		CMD_handle(RxMsg);
	}
}
bool iRDM::UDP_send(const MSG_PKG& msg)
{
	if (udpSocket && udpSocket->writeDatagram((char *)&msg.cmd_pkg, sizeof(msg.cmd_pkg), msg.rIP, msg.rPort) != sizeof(msg.cmd_pkg))
	{
		return false;
	}
	return true;
}
bool iRDM::TCP_send(const MSG_PKG& msg)
{
	if (m_tcpServerConnection->state() != QAbstractSocket::ConnectedState) return false;

	QByteArray outBlock;
	outBlock.append((char*)&msg.cmd_pkg, sizeof(msg.cmd_pkg));
	
	if (m_tcpServerConnection->write(outBlock) < 0) return false;
	else 	return true;	
}
void iRDM::CMD_handle(const MSG_PKG& msg)
{
	if (msg.cmd_pkg.header.ind != UDP_IND) return;

	switch (msg.cmd_pkg.header.cmd )
	{
		case UDP_DISCOVER:
		{
			MSG_PKG txMsg;
			txMsg.cmd_pkg.header.ind = UDP_IND;
			txMsg.cmd_pkg.header.cmd = UDP_DISCOVER;
			//Test datas
			RDM_Paramters rdm_p;
			memset(&rdm_p, 0, sizeof(rdm_p));
			strcpy(rdm_p.RdmIp, "169.254.19.199");
			strcpy(rdm_p.RdmName, "Rdm_1");
			strcpy(rdm_p.RdmMAC, "16-92-54-19-19-99");

			//todo: send back local ip to remote
			txMsg.cmd_pkg.header.len = sizeof(rdm_p);
			memcpy(txMsg.cmd_pkg.data, &rdm_p, sizeof(rdm_p));

			txMsg.rIP = msg.rIP;
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

					savedfile= new QFile(QCoreApplication::applicationDirPath() + "/" + filename);
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

void iRDM::acceptConnection()
{
	m_tcpServerConnection = m_tcpServer.nextPendingConnection();
	connect(m_tcpServerConnection, SIGNAL(readyRead()), this, SLOT(TcpServer_readyRead()));
	connect(m_tcpServerConnection, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onTcpSocketstatechanged(QAbstractSocket::SocketState)));
	connect(m_tcpServerConnection, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onTcpError(QAbstractSocket::SocketError)));	
}
void iRDM::onTcpSocketstatechanged(QAbstractSocket::SocketState state)
{
	if (state == QAbstractSocket::UnconnectedState)
	{
		fileReceived = 0;
		filesize = 0;
		if (savedfile)
			savedfile->close();
	}
}
void iRDM::TcpServer_readyRead()
{
	if ((filesize > 0) && (!filename.isEmpty()))
	{
		//start save file			
		fileReceived += m_tcpServerConnection->bytesAvailable();
		fileblock = m_tcpServerConnection->readAll();
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
void iRDM::onTcpError(QAbstractSocket::SocketError error)
{
	qDebug() << m_tcpServerConnection->errorString();
	m_tcpServerConnection->close();
	if(savedfile)
		savedfile->close();
}
