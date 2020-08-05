#pragma once

#include <QThread>
#include <QMap>
#include "tm_reader.h"

enum
{
	PLAN_OCRSSI	= 0,
	PLAN_TEMP	= 1,
	PLAN_CNT
};
#define RD_TIMEOUT		500
#define STOP_N_TRIGGER	16


class iRDM;
class iTag;
class iReader : public QThread
{
	Q_OBJECT

public:
	iReader(QObject *parent);
	~iReader();

	bool RD_init(bool force=false);
	void RD_stop() 
	{
		bStopped = true; 
		while (isRunning());
	}
	void RD_restart()
	{
		bStopped = false;
		start();
	}
	bool wirteEpc(const QString& epc_old, const QString& epc_new);
	QString RD_ErrMsg() { return QString(TMR_strerr(tmrReader, ret)); }
	bool RD_isError() { return bError; }

	quint8 get_reader_temp() {
		return reader_temp;
	}
	int	RD_power() { return power; }
	void RD_setPower(int pwr);

protected:
	quint64 readtagTid(TMR_TagFilter *filter);
	quint64 readtagCalibration(TMR_TagFilter *filter);
	void	readcallback(TMR_Reader *reader, const TMR_TagReadData *t, void *cookie);
	void	exceptioncallback(TMR_Reader *reader, TMR_Status error, void *cookie);
	quint64 bytes2longlong(QByteArray& bytes);
	bool	switchplans();

	virtual void run();
	void	handleError();

private:
	iRDM*		RDM;
	TMR_Reader  *tmrReader;
	QString		m_uri;
	quint8		antennaList[2];
	TMR_Status  ret;
	bool		bCreated;
	bool		bStopped;
	bool		bError;
	int			power;

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

	TMR_TagOp		lock_op;
	TMR_GEN2_Password accessPassword;
	int  cur_plan;

	//Async read
	TMR_ReadListenerBlock rlb;
	TMR_ReadExceptionListenerBlock reb;

	quint8 reader_temp;


signals:
	void tagUpdated(iTag*);
};
