#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_iTmsTest.h"
#include <QModbusDataUnit> 
#include <QSerialPort>

//state machine for reading tag data
typedef enum
{
	STM_RDM_INFO=0,
	STM_RDM_SYSTIME,
	STM_TAG_CNT,																				    //read input reg   0x000F
	STM_TAG_TEMP,																					//read input regs [0x0010 - 0x003F]
	STM_TAG_RSSI,																					//read input regs [0x0040 - 0x006F]
	STM_TAG_OCRSSI,																					//read input regs [0x0070 - 0x009F]
	STM_TAG_EPC,																					//read input regs [0x0160 - 0x02D8]
	//STM_TAG_ONLINE,																					//read discrete regs [0x0000 - 0x002F]
	STM_TAG_ALARM,																					//read discrete regs [0x0030 - 0x005F]
	STM_TAG_TEMPLIMIT,
	STM_TAG_END
}STM_TAG;


#ifndef		LOBYTE
#define		LOBYTE(word)			((word)&0xFF)
#endif

#ifndef		HIBYTE
#define		HIBYTE(word)			(((word)>>8)&0xFF)
#endif


#define ADDRESS_TAGCOUNT		   0x000F



#define STARTADDRESS_RDMSYSTIME    0x0000
#define STARTADDRESS_RDMINFO	   0x0003				//info means Rdm name and version

#define STARTADDRESS_TEMPERATURE   0x0010
#define STARTADDRESS_RSSI		   0x0040
#define STARTADDRESS_OCRSSI		   0x0070
#define STARTADDRESS_EPC		   0x0160


#define STARTADDRESS_TEMPLIMIT     0x0010

#define STARTADDRESS_ONLINE		   0x0000
#define STARTADDRESS_ALARM		   0x0030



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
	bool openComm(int baurate, int flow_ctrl, int databits, int stopbits, int parity);
	void DB_clearTags();																			//clear table 'TAGS'

	void read();
	QModbusDataUnit readRequest() const;

	void dataHandler(QModbusDataUnit unit);
	void insertNewTag2DB(int cnt);


private:
	Ui::iTmsTestClass ui;

	QSerialPort*	serial;
	QModbusClient*	modbus;
	QSqlTableModel*	tagModel;
	QSqlTableModel*	dataModel;

	int m_nTimerId_200ms;																			//time to read one kind of tag data
	int m_nTimerId_5s;																				//time to refresh tags from table 'TAGS'
	int	m_nTimerId_5min;																			//time to copy table 'TAGS' into table 'DATA'
	int m_nTagStm;
	int m_CurrentTagcnt;

	QMap<int, QSerialPort::Parity> paritymap;

	void saveHistorydata();
	QString m_RdmName;
	QModbusDataUnit writeRequest;
	void modbuswrite();

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
	void OnTagDataSelectChanged(const QModelIndex &, const QModelIndex &);
};
