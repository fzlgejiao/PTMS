#include "CModbus.h"
#include "irdm.h"
#include <QModbusRtuSerialSlave>
#include <QModbusTcpServer>

#if __linux__
#include <time.h>
#endif

CModbus::CModbus(QObject *parent)
	: QObject(parent)
{
	//close the Modbus's warning message output
	QLoggingCategory::setFilterRules(QStringLiteral("qt.modbus.warning=false"));

	RDM= (iRDM *)parent;
	
	modbusDevice	= NULL;	
	m_connectiontype= NONE;

	m_rtucomname	= "";
	m_rtuslaveaddress	= 0;
	m_rtubaudrate	= RTU_BAUDRATE::BaudUnknow;
	m_rtuparity		= QSerialPort::UnknownParity;

	baudratemap.clear();
	baudratemap.insert(Baudrate1200, QSerialPort::Baud1200);
	baudratemap.insert(Baudrate2400,QSerialPort::Baud2400);
	baudratemap.insert(Baudrate4800,QSerialPort::Baud4800);
	baudratemap.insert(Baudrate9600,QSerialPort::Baud9600);
	baudratemap.insert(Baudrate19200,QSerialPort::Baud19200);
	baudratemap.insert(Baudrate38400,QSerialPort::Baud38400);
	baudratemap.insert(Baudrate57600,QSerialPort::Baud57600);
	baudratemap.insert(Baudrate115200,QSerialPort::Baud115200);
}
CModbus::~CModbus()
{
	if (modbusDevice)
		modbusDevice->disconnectDevice();
	delete modbusDevice;
}
bool CModbus::MB_init()
{
	if (RDM->modbustype == "RTU")
		return MB_initRTU(RDM->rtucomname, RDM->rtuslaveaddress, RDM->rtubaudrate, (QSerialPort::Parity)RDM->rtuparity);
	else// if (RDM->modbustype == "TCP")
		return MB_initTCP(RDM->RDM_ip, RDM->TcpPort, RDM->rtuslaveaddress);
}
bool CModbus::MB_initRTU(QString comname, quint8 slaveaddress, int baudrate, QSerialPort::Parity parity)
{
	if (modbusDevice)
	{
		//if same device, no need to create again
		if (comname == m_rtucomname
			&& slaveaddress == m_rtuslaveaddress
			&& baudrate == m_rtubaudrate
			&& parity == m_rtuparity)
			return true;
		modbusDevice->disconnectDevice();
		delete modbusDevice;
		modbusDevice = NULL;
	}

	RTU_BAUDRATE baud = baudratemap.key((QSerialPort::BaudRate)baudrate, BaudUnknow);
	if (baud == BaudUnknow) return false;

	if ((parity != QSerialPort::EvenParity) && (parity != QSerialPort::OddParity) && (parity != QSerialPort::NoParity))		
		return false;

	m_connectiontype = RTU;
	modbusDevice = new QModbusRtuSerialSlave(this);					//Use Modbus-RTU slave mode

	modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter, comname);
	modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter, parity);
	
	modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, baudrate);
	modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, QSerialPort::Data8);
	modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, QSerialPort::OneStop);

	m_rtubaudrate	= baud;
	m_rtuparity		= parity;
	m_rtucomname	= comname;
	m_rtuslaveaddress = slaveaddress;

	modbusDevice->setServerAddress(slaveaddress);
	setModBusfilter();
		
	setupDeviceData();

	connect(modbusDevice, &QModbusServer::stateChanged, this, &CModbus::onStateChanged);
	connect(modbusDevice, &QModbusServer::dataWritten, this, &CModbus::onReceivedWritten);
	connect(modbusDevice, &QModbusServer::errorOccurred, this, &CModbus::handleDeviceError);

	return modbusDevice->connectDevice();	
}
bool CModbus::MB_initTCP(QString ipaddress, int port, quint8 slaveaddress)
{
	if (modbusDevice)
	{
		//if same device, no need to create again
		if (ipaddress == m_ipaddress
			&& slaveaddress == m_rtuslaveaddress)
			return true;

		modbusDevice->disconnectDevice();
		delete modbusDevice;
		modbusDevice = NULL;
	}

	m_connectiontype = TCP;
	m_rtuslaveaddress = slaveaddress;
	m_ipaddress = ipaddress;

	modbusDevice = new QModbusTcpServer(this);						//use modbus tcp mode
	setModBusfilter();

	modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, ipaddress);
	modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, port);
	modbusDevice->setServerAddress(slaveaddress);

	setupDeviceData();

	connect(modbusDevice, &QModbusServer::stateChanged, this, &CModbus::onStateChanged);
	connect(modbusDevice, &QModbusServer::dataWritten, this, &CModbus::onReceivedWritten);
	connect(modbusDevice, &QModbusServer::errorOccurred, this, &CModbus::handleDeviceError);

	return modbusDevice->connectDevice();
}
void CModbus::setModBusfilter()
{
	if (!modbusDevice) return;
	//set receive data filter and address range
	QModbusDataUnitMap reg;
	reg.insert(QModbusDataUnit::Coils, { QModbusDataUnit::Coils, Start_Coils, Count_Coils });
	reg.insert(QModbusDataUnit::DiscreteInputs, { QModbusDataUnit::DiscreteInputs, Start_DiscreteInputs, Count_DiscreteInputs });
	reg.insert(QModbusDataUnit::InputRegisters, { QModbusDataUnit::InputRegisters, Start_InputRegister, Count_InputRegister });
	reg.insert(QModbusDataUnit::HoldingRegisters, { QModbusDataUnit::HoldingRegisters, Start_HoldingRegister, Count_HoldingRegister });
	modbusDevice->setMap(reg);
}
void CModbus::onStateChanged(QModbusDevice::State state)
{
	m_status = state;
	if (state == QModbusDevice::ConnectedState)
	{
	}
}
void CModbus::handleDeviceError(QModbusDevice::Error newError)
{

}
void CModbus::onReceivedWritten(QModbusDataUnit::RegisterType table, int address, int size)
{		
	for (int i = 0; i < size; ++i) {
		quint16 value;
		switch (table) {
		case QModbusDataUnit::Coils:
			modbusDevice->data(QModbusDataUnit::Coils, address + i, &value);			
			handler_coils(address + i, value);
			break;
		case QModbusDataUnit::HoldingRegisters:
			modbusDevice->data(QModbusDataUnit::HoldingRegisters, address + i, &value);
			handler_holdingRegister(address + i, value);
			break;
		default:
			break;
		}
	}
}

void CModbus::setupDeviceData()
{
	//HoldingRegisters datas
	modbusDevice->setData(QModbusDataUnit::HoldingRegisters, HoldingRegister_RTU, B2WORD(m_rtubaudrate, m_rtuparity));
	modbusDevice->setData(QModbusDataUnit::HoldingRegisters, HoldingRegister_TCPPORT, ModbusTcpPort);

	//InputRegister datas,save tag count
	modbusDevice->setData(QModbusDataUnit::InputRegisters, InputRegister_TagCOUNT, RDM->Tag_count());

	int sid = 0;
	for (int i = 0; i < RDM->Tag_count(); i++)
	{
		sid++;
		/* save all tags upper limit*/
		quint16 address = HoldingRegister_Tag1UPPERLIMIT;
		iTag *tag = RDM->Tag_getbysid(sid);
		if (!tag) continue;

		modbusDevice->setData(QModbusDataUnit::HoldingRegisters, address + i, tag->T_uplimit);

		/* save all tags UID*/
		address = InputRegister_Tag1UID;
		quint64 uid = tag->T_uid;
		if (uid != 0)
		{
			int offset = 0;
			modbusDevice->setData(QModbusDataUnit::InputRegisters, address + 4 * i + offset, HI1WORD(uid));
			offset++;
			modbusDevice->setData(QModbusDataUnit::InputRegisters, address + 4 * i + offset, HI2WORD(uid));
			offset++;
			modbusDevice->setData(QModbusDataUnit::InputRegisters, address + 4 * i + offset, LO1WORD(uid));
			offset++;
			modbusDevice->setData(QModbusDataUnit::InputRegisters, address + 4 * i + offset, LO2WORD(uid));
		}
		/* save all tags EPC*/
		address = InputRegister_Tag1EPC;
		QString  epc = tag->T_epc;
		QByteArray bytelist = epc.toUtf8();
		quint8 wordlength = epc.length() / 2;
		
		if (wordlength <= 8)		 	//valid check
		{
			//first clear all modbus epc data
			for (int j = 0; j < 8; j++)
				modbusDevice->setData(QModbusDataUnit::InputRegisters, address + 8 * i + j, 0);
			for (int j = 0; j < wordlength; j++)
				modbusDevice->setData(QModbusDataUnit::InputRegisters, address + 8 * i + j, B2WORD(bytelist[2 * j], bytelist[2 * j + 1]));
		}
		
		/* save all tags enable status*/
		address = Coil_Tag1Enable;
		modbusDevice->setData(QModbusDataUnit::Coils, address + i, tag->T_enable);

		//DiscreteInput datas,all real time datas, not default values
		/* save all tags temp alarm*/
		/* save all tags online status*/
	}
}
bool CModbus::changeRTUserial(RTU_BAUDRATE rate, QSerialPort::Parity parity)
{
	if (m_connectiontype != RTU) return false;

	if (!modbusDevice) return false;

	if (modbusDevice->state() == QModbusDevice::ConnectedState)
		modbusDevice->disconnectDevice();
	
	modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter, parity);
	modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, baudratemap.value(rate));

	return modbusDevice->connectDevice();
}
bool CModbus::changeTCPport(int port)
{
	if (m_connectiontype != TCP) return false;

	if (!modbusDevice) return false;

	if (modbusDevice->state() == QModbusDevice::ConnectedState)
		modbusDevice->disconnectDevice();

	modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, port);

	return modbusDevice->connectDevice();
}

void CModbus::handler_coils(quint16 address,quint16 value)
{
	bool bitvalue = value;	
	
	int sid = address - Coil_Tag1Enable + 1;

	iTag *tag = RDM->Tag_getbysid(sid);
	if (tag)
		tag->T_enable = bitvalue;	
}
void CModbus::handler_holdingRegister(quint16 address, quint16 value)
{		
	bool error = false;

	switch (address)
	{
	case HoldingRegister_RTU:
	{
		quint8 newbaudrate = HIBYTE(value);
		quint8 newparity = LOBYTE(value);

		if (newbaudrate >= BaudUnknow)
			error = true;
		else if ((newparity != QSerialPort::EvenParity) && (newparity != QSerialPort::OddParity) && (newparity != QSerialPort::NoParity))
			error = true;
		else if (changeRTUserial((RTU_BAUDRATE)newbaudrate, (QSerialPort::Parity)newparity))
		{
			m_rtubaudrate = (RTU_BAUDRATE)newbaudrate;
			m_rtuparity = (QSerialPort::Parity)newparity;
		}
		else
		{
			error = true;
		}
		//when data is not valid or error, roll back the raw data
		if(error)
			modbusDevice->setData(QModbusDataUnit::HoldingRegisters, HoldingRegister_RTU, B2WORD(m_rtubaudrate, m_rtuparity));
	}
	break;

	case HoldingRegister_TCPPORT:
	{
		//ignore it,always keep default
		//if (m_connectiontype == RTU)
			modbusDevice->setData(QModbusDataUnit::HoldingRegisters, HoldingRegister_TCPPORT, ModbusTcpPort);
		//else if (m_connectiontype == TCP)
		{
		//	if (!changeTCPport(value))
			{
		//		modbusDevice->setData(QModbusDataUnit::HoldingRegisters, HoldingRegister_TCPPORT, ModbusTcpPort);
			}
		}
	}
		break;			

	case HoldingRegister_SYSYEARMONTH:
	{
		quint8 year = HIBYTE(value);
		quint8 month = LOBYTE(value);
		if ((year > 99) || (month > 12)) error = true;

		if (error)
		{
			//roll back the modbus data
			quint8 rawyear = QDateTime::currentDateTime().toString("yy").toUInt();
			quint8 rawmonth = QDateTime::currentDateTime().date().month();
			modbusDevice->setData(QModbusDataUnit::HoldingRegisters, HoldingRegister_SYSYEARMONTH, B2WORD(rawyear, rawmonth));
		}
		else
		{
			//update read only register
			modbusDevice->setData(QModbusDataUnit::InputRegisters, InputRegister_SYSYEARMONTH, B2WORD(year, month));
		
			//set system datetime
			int year_t = QString("20%1").arg(year).toInt();
			QDateTime datetime = QDateTime::currentDateTime();
			datetime.setDate(QDate(year_t, month, datetime.date().day()));
#if __linux__
			time_t newdatetime = (time_t)datetime.toTime_t();
			stime(&newdatetime);
#endif
		}
	}
	break;

	case HoldingRegister_SYSDAYHOUR:
	{
		quint8 day = HIBYTE(value);
		quint8 hour = LOBYTE(value);

		if ((day > 31) || (hour > 23)) error = true;
		if (error)
		{
			//roll back the modbus data 
			quint8 rawday = QDateTime::currentDateTime().date().day();
			quint8 rawhour = QDateTime::currentDateTime().time().hour();
			modbusDevice->setData(QModbusDataUnit::HoldingRegisters, HoldingRegister_SYSDAYHOUR, B2WORD(rawday, rawhour));
		}
		else
		{
			//update read only register
			modbusDevice->setData(QModbusDataUnit::InputRegisters, InputRegister_SYSDAYHOUR, B2WORD(day, hour));
			//set system datetime
			QDateTime datetime = QDateTime::currentDateTime();
			datetime.setDate(QDate(datetime.date().year(), datetime.date().month(), day));
			datetime.setTime(QTime(hour, datetime.time().minute(), datetime.time().second()));
#if __linux__
			time_t newdatetime = (time_t)datetime.toTime_t();
			stime(&newdatetime);
#endif
		}
	}
	break;

	case HoldingRegister_SYSMINSECOND:
	{
		quint8 minute = HIBYTE(value);
		quint8 second = LOBYTE(value);

		if ((minute > 59) || (second > 59)) error = true;

		if (error)
		{
			//roll back the modbus data
			quint8 rawminute = QDateTime::currentDateTime().time().minute();
			quint8 rawsecond = QDateTime::currentDateTime().time().second();

			modbusDevice->setData(QModbusDataUnit::HoldingRegisters, HoldingRegister_SYSMINSECOND, B2WORD(rawminute, rawsecond));
		}
		else
		{
			//update read only register
			modbusDevice->setData(QModbusDataUnit::InputRegisters, InputRegister_SYSMINSECOND, B2WORD(minute, second));
			//set system datetime
			QDateTime datetime = QDateTime::currentDateTime();	
			datetime.setTime(QTime(datetime.time().hour(), minute, second));
#if __linux__
			time_t newdatetime = (time_t)datetime.toTime_t();
			stime(&newdatetime);
#endif
		}
	}
	break;

	default:
	{
		int sid = address - HoldingRegister_Tag1UPPERLIMIT + 1;
		iTag *tag = RDM->Tag_getbysid(sid);
		if (tag)
			tag->T_uplimit = value;
	}
		break;
	}
}

void CModbus::updateRdmRegisters(iTag *tag)
{	
	if (!tag) return;
	if (m_status != QModbusDevice::ConnectedState) return;
	//update current temperature
	int sid = tag->T_sid;
	quint16 address = InputRegister_Tag1Temp + sid - 1;
	qint16 currenttemp = tag->T_temp * 10;
	modbusDevice->setData(QModbusDataUnit::InputRegisters, address, currenttemp);

	//update RSSI
	address = InputRegister_Tag1RSSI + sid - 1;
	qint16 rssi = tag->T_rssi;
	modbusDevice->setData(QModbusDataUnit::InputRegisters, address, rssi);
	
	//update Oc-RSSI
	address = InputRegister_Tag1OCRSSI + sid - 1;
	quint16 oc_rssi = tag->T_OC_rssi;
	modbusDevice->setData(QModbusDataUnit::InputRegisters, address, oc_rssi);

	//update online status
	address = DiscreteInput_Tag1Online + sid - 1;
	modbusDevice->setData(QModbusDataUnit::DiscreteInputs, address, tag->isonline());

	//update temperature alarm
	address = DiscreteInput_Tag1TempAlarm + sid - 1;
	modbusDevice->setData(QModbusDataUnit::DiscreteInputs, address, tag->isAlarm());
}
void CModbus::updatesystime(QDateTime time)
{
	if (m_status != QModbusDevice::ConnectedState) return;

	quint8 year = time.date().toString("yy").toUInt();			//get last two bits of the year
	
	quint8 month = time.date().month();
	quint8 day = time.date().day();

	quint8 hour = time.time().hour();
	quint8 min = time.time().minute();
	quint8 second = time.time().second();

	modbusDevice->setData(QModbusDataUnit::InputRegisters, InputRegister_SYSYEARMONTH, B2WORD(year,month));
	modbusDevice->setData(QModbusDataUnit::InputRegisters, InputRegister_SYSDAYHOUR, B2WORD(day, hour));
	modbusDevice->setData(QModbusDataUnit::InputRegisters, InputRegister_SYSMINSECOND, B2WORD(min, second));
}





