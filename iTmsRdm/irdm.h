#pragma once

#include <QObject>
#include "ireader.h"
#include "idevice.h"

class iRDM : public QObject
{
	Q_OBJECT

public:
	iRDM(QObject *parent=NULL);
	~iRDM();


private:
	iReader*	reader;																				//RFID reader
	iDevice		device;																				//IOT device
};
