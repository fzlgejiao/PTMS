#include "iTmsTest.h"
#include <QSerialPortInfo> 
#include <QSerialPort>
#include <QSqlTableModel> 
#include <QSqlQuery> 
#include <QModbusClient> 
#include <QModbusRtuSerialMaster> 

iTmsTest::iTmsTest(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	this->setWindowTitle(QString("PTMS - MODBUS %1").arg(qApp->applicationVersion()));

	serial = new QSerialPort(this);
	modbus = new QModbusRtuSerialMaster(this);

	ui.btnConnect->setEnabled(true);
	ui.btnDisconnect->setEnabled(false);
	OnRefresh();

	//for tags
	tagModel = new QSqlTableModel(this);
	tagModel->setTable("TAGS");
	tagModel->select();

	ui.tableViewOnline->setSortingEnabled(true);
	ui.tableViewOnline->setModel(tagModel);
	ui.tableViewOnline->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableViewOnline->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableViewOnline->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableViewOnline->setAlternatingRowColors(true);

	//for data
	dataModel = new QSqlTableModel(this);
	dataModel->setTable("DATA");
	dataModel->select();

	ui.tableViewHistory->setSortingEnabled(true);
	ui.tableViewHistory->setModel(dataModel);
	ui.tableViewHistory->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableViewHistory->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableViewHistory->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableViewHistory->setAlternatingRowColors(true);
	ui.tableViewHistory->setColumnWidth(0, 200);

	ui.tabWidget->setCurrentIndex(0);

	DB_clearTags();

	m_nTagStm = 0;
	m_nTimerId_200ms = startTimer(200);
	m_nTimerId_5s = startTimer(5000);
	m_nTimerId_5min = startTimer(300000);


	connect(modbus, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
		statusBar()->showMessage(modbus->errorString(), 5000);
	});
	connect(modbus, &QModbusClient::stateChanged,this, &iTmsTest::OnStateChanged);
}
iTmsTest::~iTmsTest()
{
	if (modbus)
		modbus->disconnectDevice();
	delete modbus;
}
void iTmsTest::OnRefresh()
{
	ui.cbxComm->clear();
	ui.cbxComm->addItems(getComms());
}
void iTmsTest::OnConnect()
{
	if (ui.cbxComm->currentIndex() != -1)
	{
		//serial->setPortName(ui.cbxComm->currentText());
		//openComm(ui.cbxBaudRate->currentText().toInt(),
		//			0,
		//			ui.cbxDataBit->currentText().toInt(),
		//			ui.cbxStopBit->currentText().toInt(),
		//			ui.cbxParityBit->currentText().toInt());

		modbus->setConnectionParameter(QModbusDevice::SerialPortNameParameter,
			ui.cbxComm->currentText());
		modbus->setConnectionParameter(QModbusDevice::SerialParityParameter,
			ui.cbxParityBit->currentText().toInt());
		modbus->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,
			ui.cbxBaudRate->currentText().toInt());
		modbus->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,
			ui.cbxDataBit->currentText().toInt());
		modbus->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,
			ui.cbxStopBit->currentText().toInt());
	
		modbus->setTimeout(500);																	//response timeout
		modbus->setNumberOfRetries(3);
		if (!modbus->connectDevice()) {
			statusBar()->showMessage(tr("Connect failed: ") + modbus->errorString(), 5000);
		}
		else {
			ui.btnConnect->setEnabled(false);
			ui.btnDisconnect->setEnabled(true);
		}
	}
}
void iTmsTest::OnDisconnect()
{
	serial->clear();
	serial->close();
	modbus->disconnectDevice();
	ui.btnConnect->setEnabled(true);
	ui.btnDisconnect->setEnabled(false);
}
bool iTmsTest::openComm(int baurate, int flow_ctrl, int databits, int stopbits, int parity)
{
	bool ret = serial->open(QIODevice::ReadWrite);
	if (ret == false)
		return false;
	//设置波特率
	switch (baurate)
	{
	case 1200:
		serial->setBaudRate(QSerialPort::Baud1200);
		break;
	case 2400:
		serial->setBaudRate(QSerialPort::Baud2400);
		break;
	case 4800:
		serial->setBaudRate(QSerialPort::Baud4800);
		break;
	case 9600:
		serial->setBaudRate(QSerialPort::Baud9600);
		break;
	case 19200:
		serial->setBaudRate(QSerialPort::Baud19200);
		break;
	case 38400:
		serial->setBaudRate(QSerialPort::Baud38400);
		break;
	case 57600:
		serial->setBaudRate(QSerialPort::Baud57600);
		break;
	case 115200:
		serial->setBaudRate(QSerialPort::Baud115200);
		break;
	default:
		serial->setBaudRate(baurate);
		break;

	}
	//设置数据控制流
	switch (flow_ctrl)   
	{
	case 0://不使用流控制
		serial->setFlowControl(QSerialPort::NoFlowControl);
		break;
	case 1://使用硬件流控制
		serial->setFlowControl(QSerialPort::HardwareControl);
		break;
	case 2://使用软件流控制
		serial->setFlowControl(QSerialPort::SoftwareControl);
		break;
	default:
		serial->setFlowControl(QSerialPort::UnknownFlowControl);
	}
	//设置数据位
	switch (databits)
	{
	case 5:
		serial->setDataBits(QSerialPort::Data5);
		break;
	case 6:
		serial->setDataBits(QSerialPort::Data6);
		break;
	case 7:
		serial->setDataBits(QSerialPort::Data7);
		break;
	case 8:
		serial->setDataBits(QSerialPort::Data8);
		break;
	default:
		serial->setDataBits(QSerialPort::UnknownDataBits);
		break;
	}
	//设置奇偶效验
	switch (parity)
	{
	case 'n':
	case 'N': //无奇偶校验位。
		serial->setParity(QSerialPort::NoParity);
		break;
	case 'o':
	case 0://设置为奇校验
		serial->setParity(QSerialPort::OddParity);
		break;
	case 'e':
	case 'E'://设置为偶校验
		serial->setParity(QSerialPort::EvenParity);
		break;
	default:
		serial->setParity(QSerialPort::UnknownParity);
		break;
	}
	//设置停止位
	switch (stopbits)
	{
	case 1:  serial->setStopBits(QSerialPort::OneStop); break;
	case 2:	 serial->setStopBits(QSerialPort::TwoStop); break;
	default: serial->setStopBits(QSerialPort::UnknownStopBits); break;
	}
	return ret;
}

QStringList iTmsTest::getComms()
{
	QStringList comms;

	QList<QSerialPortInfo>	ports = QSerialPortInfo::availablePorts();
	for (const QSerialPortInfo& port : ports)
	{
		if (port.isNull() == true)
			continue;
			
		comms.append(port.portName());
	}
	return comms;
}
void iTmsTest::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == m_nTimerId_200ms)
	{
		//time read one kind of tag data
		read();
		m_nTagStm++;
		if (m_nTagStm == STM_TAG_END)
			m_nTagStm = 0;
	}
	else if (event->timerId() == m_nTimerId_5s)
	{
		//time to refresh tags from table 'TAGS'
		tagModel->select();
	}
	else if (event->timerId() == m_nTimerId_5min)
	{
		//time to copy table 'TAGS' into table 'DATA'

		dataModel->select();
	}
}
void iTmsTest::DB_clearTags()
{
	QSqlQuery	query;
	query.exec("DELETE FROM TAGS");
}
void iTmsTest::DB_clearData()
{
	QSqlQuery	query;
	query.exec("DELETE FROM DATA");
}
void iTmsTest::OnStateChanged(int state)
{
	bool connected = (state != QModbusDevice::UnconnectedState);
	if (connected)
	{
		ui.btnConnect->setEnabled(false);
		ui.btnDisconnect->setEnabled(true);
	}
	else
	{
		ui.btnConnect->setEnabled(true);
		ui.btnDisconnect->setEnabled(false);
	}
}
void iTmsTest::read()
{
	if (modbus->state() == QModbusDevice::UnconnectedState)
		return;

	statusBar()->clearMessage();

	if (auto *reply = modbus->sendReadRequest(readRequest(), ui.leRdmRTUAddr->text().toInt())) {
		if (!reply->isFinished())
			connect(reply, &QModbusReply::finished, this, &iTmsTest::readReady);
		else
			delete reply; // broadcast replies return immediately
	}
	else {
		statusBar()->showMessage(tr("Read error: ") + modbus->errorString(), 5000);
	}
}
void iTmsTest::readReady()
{
	auto reply = qobject_cast<QModbusReply *>(sender());
	if (!reply)
		return;

	if (reply->error() == QModbusDevice::NoError) {
		const QModbusDataUnit unit = reply->result();
		for (uint i = 0; i < unit.valueCount(); i++) {
			const QString entry = tr("Address: %1, Value: %2").arg(unit.startAddress() + i)
				.arg(QString::number(unit.value(i),
					unit.registerType() <= QModbusDataUnit::Coils ? 10 : 16));
			ui.listValue->addItem(entry);
		}
	}
	else if (reply->error() == QModbusDevice::ProtocolError) {
		statusBar()->showMessage(tr("Read response error: %1 (Mobus exception: 0x%2)").
			arg(reply->errorString()).
			arg(reply->rawResult().exceptionCode(), -1, 16), 5000);
	}
	else {
		statusBar()->showMessage(tr("Read response error: %1 (code: 0x%2)").
			arg(reply->errorString()).
			arg(reply->error(), -1, 16), 5000);
	}

	reply->deleteLater();
}
QModbusDataUnit iTmsTest::readRequest() const
{
	QModbusDataUnit::RegisterType type = QModbusDataUnit::Invalid;
	int startAddress = 0;
	int numberOfEntries = 16;
	switch (m_nTagStm)
	{
	case STM_TAG_INFO://read input regs [0x0000 - 0x000F]
		type = QModbusDataUnit::InputRegisters;
		startAddress = 0x0000;
		numberOfEntries = 16;
		break;
	case STM_TAG_TEMP:
		type = QModbusDataUnit::InputRegisters;
		startAddress = 0x0010;
		numberOfEntries = 16;
		break;
	case STM_TAG_RSSI:
		type = QModbusDataUnit::InputRegisters;
		startAddress = 0x0040;
		numberOfEntries = 16;
		break;
	case STM_TAG_OCRSSI:
		type = QModbusDataUnit::InputRegisters;
		startAddress = 0x0070;
		numberOfEntries = 16;
		break;
	case STM_TAG_EPC:
		type = QModbusDataUnit::InputRegisters;
		startAddress = 0x0160;
		numberOfEntries = 16;		
		break;
	case STM_TAG_ONLINE:
		type = QModbusDataUnit::DiscreteInputs;
		startAddress = 0x0000;
		numberOfEntries = 16;
		break;
	case STM_TAG_ALARM:
		type = QModbusDataUnit::DiscreteInputs;
		startAddress = 0x0030;
		numberOfEntries = 16;
		break;
	}

    return QModbusDataUnit(type, startAddress, numberOfEntries);
}