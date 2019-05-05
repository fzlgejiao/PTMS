#include "irdm.h"

iRDM::iRDM(QObject *parent)
	: QObject(parent), iotdevice(this)
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
void iRDM::Tag_init(QList<quint64>& tids)
{
	qDeleteAll(taglist);
	taglist.clear();

	for(quint64 tid : tids)
	{
		iTag *tag = new iTag(tid, this);
		taglist.insert(tag->T_id, tag);
	}
}
