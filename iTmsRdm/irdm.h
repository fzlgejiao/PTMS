#pragma once

#include <QObject>
#include <QList>
#include "ireader.h"
#include "idevice.h"
#include "itag.h"

class iRDM : public QObject
{
	Q_OBJECT

public:
	iRDM(QObject *parent=NULL);
	~iRDM();
	iTag*	Tag_get(quint64 tid) {return  taglist.value(tid, NULL);}

protected:
	void	Tag_init(QList<quint64>& tids);

private:
	iReader*	reader;																				//RFID reader
	iDevice		iotdevice;																			//IOT device

	QMap<quint64, iTag *> taglist;																	//tag's ID string map to tag
};
