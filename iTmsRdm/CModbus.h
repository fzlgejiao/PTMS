#pragma once

#include <QObject>
#include <QModbusServer>
#include <QSerialPort>
#include "itag.h"


//QModbusDataUnit::HoldingRegisters   16-bit --> use to save parameter,can both read/write
//address from  0x0000 to 0x0032

//QModbusDataUnit::InputRegisters  16-bit --> use to save (current temperature,rssi and OC-RSSI ), can only read 
//address from 0x0000 to 0x01FF

//QModbusDataUnit::Coils  bit value --> use to save tag's enbable ,can both read/write 
//address from 0x0000 to 0x001F


//QModbusDataUnit::DiscreteInputs  bit value --> use to save tags online status, only can read 
//address from 0x0000 to 0x003F

// Details in the protocol file 

#define Start_HoldingRegister   0x0000
#define Count_HoldingRegister   0x0033

#define Start_InputRegister		0x0000
#define Count_InputRegister		0x0200

#define Start_Coils				0x0000
#define Count_Coils				0x0020

#define Start_DiscreteInputs	0x0000
#define Count_DiscreteInputs	0x0040

#ifndef		LOBYTE
#define		LOBYTE(word)			((word)&0xFF)
#endif

#ifndef		HIBYTE
#define		HIBYTE(word)			(((word)>>8)&0xFF)		
#endif

#define		B2WORD(high,low)		(((high)<<8)+((low)&0xFF))

#define     HI1WORD(ll)				(((ll)>>48)&0xFFFF)
#define     HI2WORD(ll)				(((ll)>>32)&0xFFFF)
#define     LO1WORD(ll)				(((ll)>>16)&0xFFFF)
#define     LO2WORD(ll)				((ll)&0xFFFF)


//Register defines
//HoldingRegisters:			Read/Write
#define HoldingRegister_RTU				0x0000
#define HoldingRegister_TCPPORT			0x0001
#define HoldingRegister_Tag1UPPERLIMIT	0x0010
#define HoldingRegister_Tag2UPPERLIMIT	0x0011
#define HoldingRegister_Tag3UPPERLIMIT	0x0012
#define HoldingRegister_Tag4UPPERLIMIT	0x0013
#define HoldingRegister_Tag5UPPERLIMIT	0x0014
#define HoldingRegister_Tag6UPPERLIMIT	0x0015
#define HoldingRegister_Tag7UPPERLIMIT	0x0016
#define HoldingRegister_Tag8UPPERLIMIT	0x0017
#define HoldingRegister_Tag9UPPERLIMIT	0x0018
#define HoldingRegister_Tag10UPPERLIMIT	0x0019

#define HoldingRegister_SYSYEARMONTH	0x0030
#define HoldingRegister_SYSDAYHOUR		0x0031
#define HoldingRegister_SYSMINSECOND	0x0032



//InputRegisters:			Read Only
#define InputRegister_TagCOUNT			0x01FF

#define InputRegister_SYSYEARMONTH		0x01E0
#define InputRegister_SYSDAYHOUR		0x01E1
#define InputRegister_SYSMINSECOND		0x01E2

#define InputRegister_Tag1Temp			0x0000
#define InputRegister_Tag2Temp			0x0001
#define InputRegister_Tag3Temp			0x0002
#define InputRegister_Tag4Temp			0x0003
#define InputRegister_Tag5Temp			0x0004
#define InputRegister_Tag6Temp			0x0005
#define InputRegister_Tag7Temp			0x0006
#define InputRegister_Tag8Temp			0x0007
#define InputRegister_Tag9Temp			0x0008
#define InputRegister_Tag10Temp			0x0009

#define InputRegister_Tag1RSSI			0x0020
#define InputRegister_Tag2RSSI			0x0021
#define InputRegister_Tag3RSSI			0x0022
#define InputRegister_Tag4RSSI			0x0023
#define InputRegister_Tag5RSSI			0x0024
#define InputRegister_Tag6RSSI			0x0025
#define InputRegister_Tag7RSSI			0x0026
#define InputRegister_Tag8RSSI			0x0027
#define InputRegister_Tag9RSSI			0x0028
#define InputRegister_Tag10RSSI			0x0029

#define InputRegister_Tag1OCRSSI		0x0040
#define InputRegister_Tag2OCRSSI		0x0041
#define InputRegister_Tag3OCRSSI		0x0042
#define InputRegister_Tag4OCRSSI		0x0043
#define InputRegister_Tag5OCRSSI		0x0044
#define InputRegister_Tag6OCRSSI		0x0045
#define InputRegister_Tag7OCRSSI		0x0046
#define InputRegister_Tag8OCRSSI		0x0047
#define InputRegister_Tag9OCRSSI		0x0048
#define InputRegister_Tag10OCRSSI		0x0049

#define InputRegister_Tag1UID			0x0060
#define InputRegister_Tag2UID			0x0064
#define InputRegister_Tag3UID			0x0068
#define InputRegister_Tag4UID			0x006C
#define InputRegister_Tag5UID			0x0070
#define InputRegister_Tag6UID			0x0074
#define InputRegister_Tag7UID			0x0078
#define InputRegister_Tag8UID			0x007C
#define InputRegister_Tag9UID			0x0080
#define InputRegister_Tag10UID			0x0084
#define InputRegister_Tag11UID			0x0088
#define InputRegister_Tag12UID			0x008C
#define InputRegister_Tag13UID			0x0090
#define InputRegister_Tag14UID			0x0094
#define InputRegister_Tag15UID			0x0098
#define InputRegister_Tag16UID			0x009C
#define InputRegister_Tag17UID			0x00A0
#define InputRegister_Tag18UID			0x00A4
#define InputRegister_Tag19UID			0x00A8
#define InputRegister_Tag20UID			0x00AC
#define InputRegister_Tag21UID			0x00B0
#define InputRegister_Tag22UID			0x00B4
#define InputRegister_Tag23UID			0x00B8
#define InputRegister_Tag24UID			0x00BC
#define InputRegister_Tag25UID			0x00C0
#define InputRegister_Tag26UID			0x00C4
#define InputRegister_Tag27UID			0x00C8
#define InputRegister_Tag28UID			0x00CC
#define InputRegister_Tag29UID			0x00D0
#define InputRegister_Tag30UID			0x00D4
#define InputRegister_Tag31UID			0x00D8
#define InputRegister_Tag32UID			0x00DC


#define InputRegister_Tag1EPC			0x00E0
#define InputRegister_Tag2EPC			0x00E8
#define InputRegister_Tag3EPC			0x00F0
#define InputRegister_Tag4EPC			0x00F8
#define InputRegister_Tag5EPC			0x0100
#define InputRegister_Tag6EPC			0x0108
#define InputRegister_Tag7EPC			0x0110
#define InputRegister_Tag8EPC			0x0118
#define InputRegister_Tag9EPC			0x0120
#define InputRegister_Tag10EPC			0x0128

#define InputRegister_Tag32EPC			0x01D8

//Coils:			Read/Write
#define Coil_Tag1Enable					0x0000
#define Coil_Tag2Enable					0x0001
#define Coil_Tag3Enable					0x0002
#define Coil_Tag4Enable					0x0003
#define Coil_Tag5Enable					0x0004
#define Coil_Tag6Enable					0x0005
#define Coil_Tag7Enable					0x0006
#define Coil_Tag8Enable					0x0007
#define Coil_Tag9Enable					0x0008
#define Coil_Tag10Enable				0x0009

//DiscreteInputs:	Read Only
#define DiscreteInput_Tag1Online		0x0000
#define DiscreteInput_Tag2Online		0x0001
#define DiscreteInput_Tag3Online		0x0002
#define DiscreteInput_Tag4Online		0x0003
#define DiscreteInput_Tag5Online		0x0004
#define DiscreteInput_Tag6Online		0x0005
#define DiscreteInput_Tag7Online		0x0006
#define DiscreteInput_Tag8Online		0x0007
#define DiscreteInput_Tag9Online		0x0008
#define DiscreteInput_Tag10Online		0x0009

#define DiscreteInput_Tag1TempAlarm		0x0020
#define DiscreteInput_Tag2TempAlarm		0x0021
#define DiscreteInput_Tag3TempAlarm		0x0022
#define DiscreteInput_Tag4TempAlarm		0x0023
#define DiscreteInput_Tag5TempAlarm		0x0024
#define DiscreteInput_Tag6TempAlarm		0x0025
#define DiscreteInput_Tag7TempAlarm		0x0026
#define DiscreteInput_Tag8TempAlarm		0x0027
#define DiscreteInput_Tag9TempAlarm		0x0028
#define DiscreteInput_Tag10TempAlarm	0x0029


typedef enum  {
	RTU =0,
	TCP,
	NONE
}ModbusConnection;

typedef enum {
	Baudrate1200 = 0,
	Baudrate2400,
	Baudrate4800,
	Baudrate9600,
	Baudrate19200,
	Baudrate38400,
	Baudrate57600,
	Baudrate115200,
	BaudUnknow
}RTU_BAUDRATE;

#define ModbusTcpPort 2902

class iRDM;
class CModbus : public QObject
{
	Q_OBJECT

public:
	CModbus(QObject *parent);
	~CModbus();

	bool MB_init();
	ModbusConnection connectiontype() { return m_connectiontype; }
	void updateRdmRegisters(iTag *tag);
	QModbusDevice::State status() { return m_status; }
	bool isconnected() { return m_status == QModbusDevice::ConnectedState; }
	void updatesystime(QDateTime time);

protected:
	bool MB_initRTU(QString comname, quint8 slaveaddress, int baudrate, QSerialPort::Parity parity);
	bool MB_initTCP(QString ipaddress,int port, quint8 slaveaddress);


private slots:
		void onStateChanged(QModbusDevice::State state);
		void onReceivedWritten(QModbusDataUnit::RegisterType table, int address, int size);
		void handleDeviceError(QModbusDevice::Error newError);


private:
	iRDM*				RDM;
	QModbusServer*		modbusDevice;
	ModbusConnection	m_connectiontype;
	QModbusDevice::State m_status;

	//modbus settings
	QString				m_com;
	int					m_slaveaddress;
	RTU_BAUDRATE		m_rtubaud;
	QSerialPort::Parity	m_rtuparity;
	QMap<RTU_BAUDRATE,QSerialPort::BaudRate> baudratemap;
	
	
	void setupDeviceData();
	void setModBusfilter();
	bool changeRTUserial(RTU_BAUDRATE rate, QSerialPort::Parity parity);
	bool changeTCPport(int port);

	void handler_coils(quint16 address, quint16 value);
	void handler_holdingRegister(quint16 address, quint16 value);
};
