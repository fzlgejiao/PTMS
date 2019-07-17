#pragma once
#include "idevice.h"
#include <QObject>
#include <QList>
#include <QTimerEvent> 

#define	RDM_TICKS	3		//max times for online check
#define RDM_TIMER	5000

class iReader;
class iDevice;
class iTag;
class iRDM : public QObject
{
	Q_OBJECT

public:
	static iRDM &Instance(QObject *parent = NULL)													//for singleton of iStation object
	{
		if (0 == _RDM)
		{
			_RDM = new iRDM(parent);
		}
		return *_RDM;
	}
	~iRDM();

	iTag*	Tag_get(quint64 uid) {return  taglist.value(uid, NULL);}
	iTag*	Tag_getbysid(int sid);
	int		Tag_count() { return taglist.count(); }
	void    Tmr_stop() { this->killTimer(timerId); }
	void    Tmr_start() { timerId = this->startTimer(RDM_TIMER); }

protected:

	bool	Cfg_load(const QString& xml);															//load rdm configuration from xml file
	void	Cfg_readrdm(QXmlStreamReader& xmlReader);
	void	Cfg_readcfg(QXmlStreamReader& xmlReader);
	void	Cfg_readtags(QXmlStreamReader& xmlReader);
	void	Cfg_skipUnknownElement(QXmlStreamReader& xmlReader);

	iTag*	Tag_add(int sid,quint64 uid,const QString& epc);

	virtual void timerEvent(QTimerEvent *event);

private:
	friend class iDevice;
	friend class iView;
	friend class iCfgDlg;

	iRDM(QObject *parent = NULL);																	//protected from external access
	static iRDM* _RDM;
	iReader*	reader;																				//RFID reader
	iDevice*	iotdevice;																			//IOT device

	QMap<quint64, iTag *> taglist;																	//tag's ID string map to tag
	int			timerId;
	int			RDM_ticks;

	QString		comName;

	//iot info
	QString		productkey;
	QString		devicename; 
	QString		devicesecret;
	QString		regionid;

	bool		RDM_available;
	bool		RDM_alarm;

	//RDM info
	QString		RDM_mac;
	QString		RDM_name;
};
