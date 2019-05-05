#include "irdm.h"

iRDM::iRDM(QObject *parent)
	: QObject(parent), device(this)
{
#ifdef WIN32
	reader = new iReader("tmr:///com22", this);
#endif

#ifdef __linux__
	reader = new iReader("tmr:///dev/ttyUSB0", this);
#endif
}

iRDM::~iRDM()
{
}
