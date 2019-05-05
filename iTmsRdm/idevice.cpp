#include "idevice.h"
#include "irdm.h"

iDevice::iDevice(QObject *parent)
	: QObject(parent)
{
	RDM = (iRDM *)parent;
}

iDevice::~iDevice()
{
}
