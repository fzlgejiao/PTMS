#pragma once

#include <QObject>
#include <QMap>
#include "tm_reader.h"


#define PLAN_CNT		2
#define RD_TIMEOUT		250


class iRDM;
class iTag;
class iReader : public QObject
{
	Q_OBJECT

public:
	iReader(QObject *parent);
	~iReader();

	bool RD_init();
	bool wirteEpc(const QByteArray& epc_old, const QString& epc_new);
	void readtag();
	void checkerror();

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
	TMR_Status  ret;

	//reader parameters
	QString		group;
	QString		hardware;
	QString		software;
	QString		modleversion;
		
	TMR_ReadPlan	subplan[PLAN_CNT];
	TMR_ReadPlan*	subplanPtrs[PLAN_CNT];
	TMR_ReadPlan	multiplan;

	//On-chip RSSI read plan
	TMR_TagFilter	OC_rssi_select;
	TMR_TagOp		OC_rssi_read;
	quint8			OC_rssi_mask;

	//read temperature plan
	TMR_TagFilter	tempselect;
	TMR_TagOp		tempread;

	//Async read
	TMR_ReadListenerBlock rlb;
	TMR_ReadExceptionListenerBlock reb;

signals:
	void tagUpdated(iTag*);
};
