#include "irdm.h"
#include "ireader.h"
#include "itag.h"
#include "ibc.h"
#include "iLed.h"
#include "CModbus.h"
#include <QDebug> 
#include <QtXml/QDomDocument>

iRDM::iRDM(QObject *parent)
	: QObject(parent)	
{
	modbustype = "RTU";
	rtucomname = "/dev/ttySC1";
	rtuslaveaddress = 1;
	rtubaudrate = 9600;
	rtuparity = 2;
	TcpPort = 2902;

	reader		= new iReader(this);
	iotdevice	= new iDevice(this);
	modbus		= new CModbus(this);
	bc			= new iBC(this);
	led			= new iLed(this);

	RDM_init();

	Tmr_start();
}

iRDM::~iRDM()
{
	reader->RD_stop();																				//stop thread before in ~iRDM
}
void iRDM::ERR_msg(const QString& module, const QString& error)
{
	qDebug() << qPrintable(module + " : " + error);
}
void iRDM::RDM_init()
{
#ifdef WIN32
	RDM_comname = "tmr:///com3";
#else
	RDM_comname = "tmr:///dev/ttySC0";
#endif
	QString xml = QCoreApplication::applicationDirPath() + "/iTmsRdm.xml";
	if (Cfg_load(xml) == false)
	{
		iRDM::ERR_msg("RDM", "Load xml config file failed.");
		return;
	}
#if __linux__
	RDM_comname = "tmr:///dev/ttySC0";
#endif
	iotdevice->IOT_init();
	if(modbus->MB_init() == false)																	//re-init modbus module
		iRDM::ERR_msg("[MODBUS ]", "Module init failed.");
	if(reader->RD_init() == false)																	//re-init reader when re-init rdm
		iRDM::ERR_msg("[READER ]", "reader init failed. Error : " + reader->RD_ErrMsg());


	RDM_available = false;
	RDM_ticks = RDM_TICKS;																			//for IOT online check

	//set tag flags for send the tag info to system
	for (iTag *tag : taglist)
	{
		tag->T_data_flag = Tag_UID | Tag_EPC | Tag_Upperlimit | Tag_Switch | Tag_Rssi | Tag_Temperature;
	}

	emit cfgChanged();
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
				RDM_note = xmlReader.attributes().value("note").toString();
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
				RDM_comname = xmlReader.readElementText();											//read com port

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
				rtucomname = xmlReader.attributes().value("comname").toString();
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

	qDebug() << "Cfg_readtags:";

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
				int max = TAG_T_ALM;
				sid = xmlReader.attributes().value("sid").toString().toInt();
				uid = xmlReader.attributes().value("uid").toString().toULongLong();
				epc = xmlReader.attributes().value("epc").toString();
				if(xmlReader.attributes().value("max").isEmpty() == false)
					max = xmlReader.attributes().value("max").toString().toInt();
				QString note = xmlReader.readElementText();

				//todo: load tag info
				iTag *tag = Tag_add(sid, uid, epc);
				if (tag)
				{
					tag->T_uplimit = max;
					tag->T_note = note;
				}
				qDebug() << "tag : sid = " << sid 
						<< " uid = " << uid 
					    << " epc = " << epc
						<< " note = " << note;


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
void iRDM::Cfg_changeTagUpLimit(iTag* tag, int max)
{
/*	QString xml = QCoreApplication::applicationDirPath() + "/iTmsRdm.xml";
	QFile file(xml);
	if (!file.open(QFile::ReadOnly | QFile::Text)) 
		return ;
	
	QDomDocument doc;
	if (!doc.setContent(&file, false))
		return;

	QDomElement root = doc.documentElement();
	QString name = root.nodeName();
	QDomNodeList nlist = root.elementsByTagName("tags"); 
	for (int i = 0; i < nlist.count(); i++)
	{
		QDomElement eleTags = nlist.at(i).toElement();
		QDomElement eleTag = eleTags.firstChildElement();
		while (!eleTag.isNull())
		{
			QDomAttr attrUid = eleTag.attributeNode("uid");
			name = attrUid.value();
			QString uid = QString("%1").arg(tag->T_uid, 20);
			if (attrUid.value().toULongLong() == tag->T_uid)
			{
				QDomAttr attrMax = eleTag.attributeNode("max");
				attrMax.setValue(QString::number(max));

			}
			eleTag = eleTag.nextSiblingElement();
		}
	}
	file.close();

	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return;
	//输出到文件
	QTextStream out_stream(&file);
	doc.save(out_stream, 4); //缩进4格
	file.close();
*/
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
	if (event->timerId() == tmrRDM)		//2s
	{	
		//upload tag data																				
		for (iTag* tag : taglist)
		{
			if (tag->T_enable == false)
				continue;

			tag->T_data_flag |= Tag_Online;
			if (tag->isonline())
				tag->T_data_flag |= Tag_Temperature|Tag_Rssi|Tag_Alarm;
			else
				tag->T_alarm_offline = true;														//offline

			modbus->updateRdmRegisters(tag);														//update tag data for modbus
			emit tagUpdated(tag);
		}
	}
	if (event->timerId() == tmrTime)  //1s,update modbus datetime registers
	{
		qDebug() << "[Modbus ] Status : " << modbus->status();
		modbus->updatesystime(QDateTime::currentDateTime());

		led->toggleled((int)LED_STATUS);

		if (reader->RD_isError())
		{
			RD_handleError();
		}
	}
	if (event->timerId() == tmrIOT)  //5s,update tag data to IOT
	{
		iotdevice->IOT_tick();
		iotdevice->PUB_rdm_event();
		for (iTag* tag : taglist)
		{
			if (tag->T_enable == false)
				continue;

			iotdevice->PUB_tag_data(tag);
			iotdevice->PUB_tag_event(tag);
		}
	}
}
int	iRDM::HW_ver()
{
	//todo: check hw version

	return HW_V2;
}

void iRDM::RD_handleError()
{
	qDebug() << "[RDERROR] : re-init rd and modbus now.";
	reader->RD_init(true);																			//re-init reader when error happen
	modbus->MB_init();																				//re-init modbus when error happen
}