#pragma once

#include <QObject>
#include <QMap>
#include "tm_reader.h"


#define PLAN_CNT		2
#define RD_TIMEOUT		100
typedef enum {
	PLAN_NONE	= 0,
	PLAN_CALI	= 1,
	PLAN_TEMP	= 2,
	PLAN_OCRSSI = 3,
	PLAN_NUM	= PLAN_OCRSSI
}PLAN_TYPE;


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
	void moveNextPlan();
	void startReading();
	void stopReading();
	void callbackCalibration(const QString& epc, qint32 rssi,quint64 calibration);
	void callbackTempCode(const QString& epc, qint32 rssi, ushort tempCode);
	void callbackOCRSSI(const QString& epc, qint32 rssi, qint8 ocrssi);

	PLAN_TYPE	tPlan;

protected:
	quint64 readtagTid(TMR_TagFilter *filter);
	quint64 readtagCalibration(TMR_TagFilter *filter);

	bool	switchplans();


private:
	iRDM*		RDM;
	TMR_Reader  *tmrReader;
	QString		m_uri;
	quint8		antennaCount;
	quint8		antennaList[2];
	TMR_Status  ret;
	bool		bCreated;

	//reader parameters
	QString		group;
	QString		hardware;
	QString		software;
	QString		modleversion;
		
	TMR_ReadPlan	subplan[PLAN_CNT];
	TMR_ReadPlan*	subplanPtrs[PLAN_CNT];
	TMR_ReadPlan	multiplan;

	TMR_TagFilter	rssiFilter;
	TMR_TagOp		rssiOP;
	TMR_TagFilter	tempFilter;
	TMR_TagOp		tempOP;
	TMR_ReadListenerBlock rlb1;
	TMR_ReadExceptionListenerBlock reb1;
	int  cur_plan;

signals:
	void tagUpdated(iTag*);
};
