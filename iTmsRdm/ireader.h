#pragma once

#include <QObject>
#include <QMap>
#include "tm_reader.h"


#define SubPlanCnt 2


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
	bool	init();
	quint64 readtagTid(TMR_TagFilter *filter);
	quint64 readtagCalibration(TMR_TagFilter *filter);
	void	readcallback(TMR_Reader *reader, const TMR_TagReadData *t, void *cookie);
	void	exceptioncallback(TMR_Reader *reader, TMR_Status error, void *cookie);
	quint64 bytes2longlong(QByteArray& bytes);
	void checkerror();

private:
	iRDM*		RDM;
	TMR_Reader  *tmrReader;
	QString		m_uri;
	quint8		antennaList[2];
	TMR_Status  ret;

	//reader parameters
	QString		group;
	QString		hardware;
	QString		software;
	QString		modleversion;
		
	TMR_ReadPlan subplan[SubPlanCnt];
	TMR_ReadPlan* subplanPtrs[SubPlanCnt];
	TMR_ReadPlan multiplan;
	//On-chip RSSI read plan
	TMR_TagFilter OC_rssi_select;
	TMR_TagOp OC_rssi_read;
	quint8 OC_rssi_mask;
	//read temperature plan
	TMR_TagFilter tempselect;
	TMR_TagOp	tempread;
	//Async read
	TMR_ReadListenerBlock rlb;
	TMR_ReadExceptionListenerBlock reb;
};
