#pragma once

#include <QObject>
#include <QList>
#include <QTimerEvent> 
#include "ireader.h"
#include "idevice.h"
#include "itag.h"

class iRDM : public QObject
{
	Q_OBJECT

public:
	iRDM(QObject *parent=NULL);
	~iRDM();
	iTag*	Tag_get(quint64 uid) {return  taglist.value(uid, NULL);}

protected:
	bool	Cfg_load(const QString& xml);															//load rdm configuration from xml file
	void	Cfg_readrdm(QXmlStreamReader& xmlReader);
	void	Cfg_readcfg(QXmlStreamReader& xmlReader);
	void	Cfg_readtags(QXmlStreamReader& xmlReader);
	void	Cfg_skipUnknownElement(QXmlStreamReader& xmlReader);

	void	Tag_add(int sid,quint64 uid,const QString& epc);

	virtual void timerEvent(QTimerEvent *event);

private:
	friend class iDevice;
	iReader*	reader;																				//RFID reader
	iDevice*	iotdevice;																			//IOT device

	QMap<quint64, iTag *> taglist;																	//tag's ID string map to tag
	int			timerId_2s;

	QString		comName;

	//iot info
	QString		productkey;
	QString		devicename; 
	QString		devicesecret;
	QString		regionid;

	bool		RDM_available;
	bool		RDM_alarm;

};
