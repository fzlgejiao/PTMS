#include "ireader.h"

iReader::iReader(QString comName, QObject *parent)
	: QObject(parent)
{
	m_uri = comName;

	TMR_Status ret;
	tmrReader = new TMR_Reader();
	ret = TMR_create(tmrReader, m_uri.toStdString().c_str());

}

iReader::~iReader()
{
}
