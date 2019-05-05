#pragma once

#include <QObject>
#include <QMap>
#include "tm_reader.h"

class iReader : public QObject
{
	Q_OBJECT

public:
	iReader(QString comName, QObject *parent);
	~iReader();


private:
	TMR_Reader  *tmrReader;
	QString		m_uri;
	quint8		antennaList[2];
};
