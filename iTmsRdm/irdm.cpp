#include "irdm.h"

iRDM::iRDM(QObject *parent)
	: QObject(parent)
{
	QString comName;
#ifdef WIN32
	comName = "tmr:///com3";
#else
	comName = "tmr:///dev/ttyUSB0";
#endif

	reader		= new iReader(comName, this);
	iotdevice	= new iDevice(this);

	QString xml;
	Cfg_load(xml);
	timerId_2s	= startTimer(2000);
}

iRDM::~iRDM()
{
}
bool iRDM::Cfg_load(const QString& xml)
{
	//parse xml 


	//todo: load iot info
	productkey	= "a19j8IwnnuH";
	devicename	= "4zDwkhgFGGSIYzTuiwkE";
	devicesecret= "tjtPWptju0WnDefExfSIGEwEodkAbwBj";
	regionid	= "cn-shanghai";
	iotdevice->IOT_init();

	//reset tag list
	qDeleteAll(taglist);
	taglist.clear();

	int sid;
	quint64 uid = 1;
	QString epc;
	//todo: load tag info
	Tag_add(sid, uid, epc);

	return true;
}
void iRDM::Tag_add(int sid, quint64 uid, const QString& epc)
{
	iTag *tag = new iTag(sid,uid,epc, this);
	if(tag)
		taglist.insert(tag->T_uid, tag);

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
			iotdevice->PUB_tag_data(tag);
			iotdevice->PUB_tag_event(tag);

		}
	}
}
