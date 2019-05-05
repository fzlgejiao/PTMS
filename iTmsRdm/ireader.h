#pragma once

#include <QObject>
#include <QMap>
#include "tm_reader.h"

class iRDM;
class iTag;
class iReader : public QObject
{
	Q_OBJECT

public:
	iReader(QString comName, QObject *parent);
	~iReader();

	bool wirteEpc(iTag *tag, QString epcstr);
	void readtag();

protected:
	quint64 readtagTid(TMR_TagFilter *filter);
	quint64 readtagCalibration(TMR_TagFilter *filter);
	void	readcallback(TMR_Reader *reader, const TMR_TagReadData *t, void *cookie);
	void	exceptioncallback(TMR_Reader *reader, TMR_Status error, void *cookie);
	quint64 bytes2longlong(QByteArray& bytes);

private:
	iRDM*		RDM;
	TMR_Reader  *tmrReader;
	QString		m_uri;
	quint8		antennaList[2];


	//reader parameters
	QString		group;
	QString		hardware;
	QString		software;
	QString		modleversion;

	//read temperature
	TMR_ReadPlan plan;
	TMR_TagFilter tempselect;
	TMR_TagOp	tempread;
	TMR_ReadListenerBlock rlb;
	TMR_ReadExceptionListenerBlock reb;
};
