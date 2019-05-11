#include "irdm.h"
#include <QDebug> 

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

	iotdevice->IOT_init();

	timerId_2s	= startTimer(2000);
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
				QString mac = xmlReader.attributes().value("mac").toString();
				QString name = xmlReader.attributes().value("name").toString();

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
				sid = xmlReader.attributes().value("sid").toString().toInt();
				uid = xmlReader.attributes().value("uid").toString().toULongLong();
				epc = xmlReader.attributes().value("epc").toString();			

				//todo: load tag info
				Tag_add(sid, uid, epc);

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
void iRDM::Tag_add(int sid, quint64 uid, const QString& epc)
{
	iTag *tag = new iTag(sid,uid,epc, this);
	if (tag)
	{
		taglist.insert(tag->T_uid, tag);
	}

}
void iRDM::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == timerId_2s)
	{
		//read tags
		reader->readtag();

		//upload rdm data
		iotdevice->PUB_rdm_event();


		//upload tag data																				
		for (iTag* tag : taglist)
		{
			if (tag->T_enable == false)
				continue;

			if(tag->isonline())
				iotdevice->PUB_tag_data(tag);
			iotdevice->PUB_tag_event(tag);
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
		}
	}
}
