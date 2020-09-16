#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_iTmsTest.h"
#include <QModbusDataUnit> 
#include <QSerialPort>
#include <QObject>
#include <QMap>
#include <QQueue> 
#include <QSqlQuery> 

//state machine for reading tag data
typedef enum
{
	STM_RDM_INFO=0,
	STM_RDM_SYSTIME,
	STM_RDM_READERTEMP,
	STM_RDM_POWER,																					//read holding reg 0x0002
	STM_TAG_CNT,																				    //read input reg   0x000F
	STM_TAG_TEMP,																					//read input regs [0x0010 - 0x003F]
	STM_TAG_RSSI,																					//read input regs [0x0040 - 0x006F]
	STM_TAG_OCRSSI,																					//read input regs [0x0070 - 0x009F]
	STM_TAG_EPC,																					//read input regs [0x0160 - 0x02D8]
	STM_TAG_ONLINE,																					//read discrete regs [0x0000 - 0x002F]
	STM_TAG_TEMPLIMIT,																				//read holding regs [0x0010 - 0x003F]
	STM_TAG_ALARM,																					//read discrete regs [0x0030 - 0x005F]
	STM_TAG_END
}STM_TAG;


#ifndef		LOBYTE
#define		LOBYTE(word)			((word)&0xFF)
#endif

#ifndef		HIBYTE
#define		HIBYTE(word)			(((word)>>8)&0xFF)
#endif




//input registers
#define STARTADDRESS_RDMSYSTIME    0x0000
#define STARTADDRESS_RDMINFO	   0x0003				//info means Rdm name and version
#define STARTADDRESS_READERTEMP	   0x000C				
#define ADDRESS_TAGCOUNT		   0x000F

#define STARTADDRESS_TEMPERATURE   0x0010
#define STARTADDRESS_RSSI		   0x0040
#define STARTADDRESS_OCRSSI		   0x0070
#define STARTADDRESS_EPC		   0x0160

//holding registers
#define STARTADDRESS_POWER		   0x0002
#define STARTADDRESS_TEMPLIMIT     0x0010

//discrete input registers
#define STARTADDRESS_ONLINE		   0x0000
#define STARTADDRESS_ALARM		   0x0030

class iTag : public QObject
{
	Q_OBJECT

public:
	iTag(int sid, QObject *parent = NULL) 
	{
		T_sid = sid; 
		T_epc = " ";
		T_rssi = 0;
		T_OC_rssi = 0;
		T_temp = 0.0;
		T_enable = true;
		T_online = false;
		T_alarm = false;
		T_uplimit = 0;
	}
	~iTag() {}
	QString Temp() { return T_online ? QString("%1").arg(T_temp, 0, 'f', 1) : "--.-"; }
	QString RSSI() { return T_online ? QString("%1").arg(T_rssi) : "---"; }
	QString OCRSSI() { return T_online ? QString("%1").arg(T_OC_rssi) : "---"; }


	bool			T_enable;
	int				T_sid;																			//squence id of tag[1 - ], ordered by uid
	bool			T_online;																		//online alarm
	bool			T_alarm;																		//temperature out of range alarm
	QString			T_epc;																			//epc 
	QString			T_note;																			//note
	qint16			T_rssi;																			//RSSI
	quint16			T_OC_rssi;																		//On-chip RSSI
	float			T_temp;																			//temperature
	int				T_uplimit;																		//up limit for temperature
	QQueue<float>	listTemps;
};

class QSerialPort;
class QSqlTableModel;
class QModbusClient;
class iTmsTest : public QMainWindow
{
	Q_OBJECT

public:
	iTmsTest(QWidget *parent = Q_NULLPTR);
	~iTmsTest();

protected:
	virtual void timerEvent(QTimerEvent *event);
	QStringList getComms();
	void DB_clearTags();																			//clear table 'TAGS'
	void DB_createTags(int cnt);
	void DB_saveTags();
	void DB_saveHistory();
	void DB_saveHistory(QSqlQuery&	query,iTag *tag,const QString& time);

	void read();
	QModbusDataUnit readRequest() const;

	void dataHandler(QModbusDataUnit unit);
	iTag* getTag(int sid) { return listTags.value(sid, NULL); }
	void createStatusBar();
private:
	Ui::iTmsTestClass ui;

	QLabel*			sCompiled;
	QModbusClient*	modbus;
	QSqlTableModel*	tagModel;
	QSqlTableModel*	dataModel;

	int m_nTimerId_200ms;																			//time to read one kind of tag data
	int m_nTimerId_1s;
	int m_nTimerId_5s;																				//time to refresh tags from table 'TAGS'
	int m_nTimerId_30s;																				//time to monitor temp change over [-2,2]
	int	m_nTimerId_5min;																			//time to copy table 'TAGS' into table 'DATA'
	int m_nTagStm;
	int m_CurrentTagcnt;

	QMap<int, QSerialPort::Parity> paritymap;


	QString m_RdmName;
	QModbusDataUnit writeRequest;
	void modbuswrite();

	int request_cnt;
	int reply_cnt;
	QMap<int, iTag *> listTags;

public slots:
	void OnRefresh();
	void OnConnect();
	void OnDisconnect();
	void OnStateChanged(int state);
	void readReady();
	void DB_clearData();																			//clear table 'DATA'
	void onSetTempLimit();
	void OnCurrentChanged();
	void OnDeleteData();
	void OnRefreshData();
	void OnTagDataSelectChanged(const QModelIndex &, const QModelIndex &);
	void OnCfgTagChanged(int);
	void OnPowerChanged(int);
	void OnSetRdPower();
};
