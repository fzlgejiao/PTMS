#include "irdm.h"
#include "ireader.h"
#include "itag.h"
#include "iLed.h"
#include "CModbus.h"
#include <QDebug> 

iRDM *iRDM::_rdm = 0;
iRDM::iRDM(QObject *parent)
	: QObject(parent)	
{
	modbustype = "RTU";
	rtucomname = "/dev/ttySC1";
	rtuslaveaddress = 1;
	rtubaudrate = 9600;
	rtuparity = 2;
	TcpPort = 2902;

	memset(&tagWrite, 0, sizeof(Tag_epc));

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
}
void iRDM::ERR_msg(const QString& module, const QString& error)
{
	qDebug() << module + " : " + error;
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
	if(modbus->MB_init() == false)
		iRDM::ERR_msg("MODBUS", "Module init failed.");
	if (reader->RD_init() == false)
		reader->checkerror();

	RDM_available = false;
	RDM_ticks = RDM_TICKS;																			//for IOT online check

	//set tag flags for send the tag info to system
	for (iTag *tag : taglist)
	{
		tag->T_data_flag = Tag_EPC | Tag_Upperlimit | Tag_Switch | Tag_Rssi | Tag_Temperature;
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
				QString epc;
				int enable;
				int max = TAG_T_MAX;
				sid = xmlReader.attributes().value("sid").toString().toInt();
				epc = xmlReader.attributes().value("epc").toString();
				if(xmlReader.attributes().value("max").isEmpty() == false)
					max = xmlReader.attributes().value("max").toString().toInt();
				QString note = xmlReader.readElementText();

				//todo: load tag info
				iTag *tag = Tag_add(sid, epc);
				if (tag)
				{
					tag->T_uplimit = max;
					tag->T_note = note;
				}
				qDebug() << "tag : sid = " << sid 
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
iTag* iRDM::Tag_add(int sid, const QString& epc)
{
	iTag *tag = new iTag(sid,epc, this);
	if (tag)
	{
		taglist.insert(epc, tag);
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
	static int count = 0;
	if (event->timerId() == tmrRDM)																	//100ms
	{

		//read/write tags data
		if (count % 2 == 0)
		{
			if (tagWrite.uid != 0)																	//write tag epc if needed
			{
				tagWrite.uid = 0;
				QByteArray epc_old = tagOnline.value(tagWrite.uid);
				reader->wirteEpc(epc_old, tagWrite.epc);
			}
			else
			{
				reader->moveNextPlan();
				reader->startReading();
			}

		}
		else
			reader->stopReading();

		count++;
		
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
						<< " epc = " << tag->T_epc 
						<< " Alarm : Offline";
					tag->T_alarm_offline = true;													//offline

					emit tagLost(tag);
				}
			}
			modbus->updateRdmRegisters(tag);
		}
	}
	if (event->timerId() == tmrTime)  //update modbus datetime registers
	{
		for (iTag* tag : taglist)
		{
			emit tagUpdated(tag);
		}
		modbus->updatesystime(QDateTime::currentDateTime());

		led->toggleled((int)LED_STATUS);
	}
	if (event->timerId() == tmrIOT)  //update tag data to IOT ,5s
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

