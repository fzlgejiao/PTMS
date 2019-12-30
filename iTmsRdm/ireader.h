#pragma once

#include <QObject>
#include <QMap>
#include "tm_reader.h"


#define RD_TIMEOUT		100

typedef enum {
	PLAN_NONE	= 0,
	PLAN_CALI	= 1,
	PLAN_TEMP	= 2,
	PLAN_OCRSSI = 3,
	PLAN_TID	= 4,
	PLAN_NUM	= PLAN_TID
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

	void checkerror();
	void moveNextPlan();
	void startReading();
	void stopReading();
	void callbackCalibration(const QString& epc, qint32 rssi,quint64 calibration);
	void callbackTempCode(const QString& epc, qint32 rssi, ushort tempCode);
	void callbackOCRSSI(const QString& epc, qint32 rssi, qint8 ocrssi);
	void callbackTid(const QString& epc, qint32 rssi, qint64 tid);

	PLAN_TYPE	tPlan;

protected:



private:
	iRDM*		RDM;
	TMR_Reader  *tmrReader;
	QString		m_uri;
	quint8		antennaCount;
	quint8		antennaList[2];
	TMR_Status  ret;
	bool		bCreated;

	TMR_ReadPlan	plan;
	TMR_TagFilter	filter;
	TMR_TagOp		tagop;
	TMR_ReadListenerBlock rlb;
	TMR_ReadExceptionListenerBlock reb;

	//reader parameters
	QString		group;
	QString		hardware;
	QString		software;
	QString		modleversion;

signals:
	void tagUpdated(iTag*);
};
