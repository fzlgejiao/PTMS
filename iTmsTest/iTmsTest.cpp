#include "iTmsTest.h"
#include <QSerialPortInfo> 
#include <QSqlTableModel> 
#include <QSqlQuery> 
#include <QModbusClient> 
#include <QModbusRtuSerialMaster> 
#include <QSqlRecord>
#include <QDebug>
#include <QSqlError>
#include <QDateTime>
#include <QSortFilterProxyModel>

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

	paritymap.insert(0, QSerialPort::NoParity);
	paritymap.insert(1, QSerialPort::OddParity);
	paritymap.insert(2, QSerialPort::EvenParity);

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

	connect(ui.tableViewOnline->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(OnCurrentChanged()));
	/*
	QSortFilterProxyModel *proxy = static_cast<QSortFilterProxyModel*>(ui.tableViewOnline->model());
	QItemSelectionModel *selectionModel = ui.tableViewOnline->selectionModel();

	QModelIndexList indexes = selectionModel->selectedRows();
	QModelIndex index;

	foreach(index, indexes) {
		int row = proxy->mapToSource(index).row();
		tagModel->removeRows(row, 1, QModelIndex());
	}
	*/
	writeRequest = QModbusDataUnit(QModbusDataUnit::Invalid, 0, 1);

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
	m_CurrentTagcnt = 0;
	m_nTimerId_200ms = startTimer(200);
	m_nTimerId_5s = startTimer(5000);
	m_nTimerId_5min = startTimer(300000);

	onSetTempLimit();

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
		
		modbus->setConnectionParameter(QModbusDevice::SerialParityParameter, paritymap.value(ui.cbxParityBit->currentIndex()));
		modbus->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,
			ui.cbxBaudRate->currentText().toInt());
		modbus->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,
			ui.cbxDataBit->currentText().toInt());
		modbus->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,
			ui.cbxStopBit->currentText().toInt());
	
		modbus->setTimeout(500);																	//response timeout
		modbus->setNumberOfRetries(0);
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
		//polling write request firstly
		if (writeRequest.registerType()!= QModbusDataUnit::Invalid) {
			modbuswrite();
			writeRequest.setRegisterType(QModbusDataUnit::Invalid);
			return;
		}
		//time read one kind of tag data
		read();
		m_nTagStm++;
		if (m_nTagStm == STM_TAG_END)
			m_nTagStm = 0;
	}
	else if (event->timerId() == m_nTimerId_5s)
	{
		//time to refresh tags from table 'TAGS'
		//tagModel->select();
	}
	else if (event->timerId() == m_nTimerId_5min)
	{
		//time to copy table 'TAGS' into table 'DATA'
		saveHistorydata();
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

	dataModel->select();
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

	//qDebug() << "Reply error="<<reply->error();

	if (reply->error() == QModbusDevice::NoError) {
		const QModbusDataUnit unit = reply->result();
		for (uint i = 0; i < unit.valueCount(); i++) {
			const QString entry = tr("Address: 0x%1, Value: %2\n").arg((unit.startAddress() + i),4,16,QChar('0'))
				.arg(QString::number(unit.value(i),
					unit.registerType() <= QModbusDataUnit::Coils ? 10 : 16));
			ui.plainTextEdit->appendPlainText(entry);
		}
		dataHandler(unit);
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
	case STM_RDM_INFO:
		type = QModbusDataUnit::InputRegisters;
		startAddress = STARTADDRESS_RDMINFO;
		numberOfEntries = 9;
		break;
	case STM_RDM_SYSTIME:
		type = QModbusDataUnit::InputRegisters;
		startAddress = STARTADDRESS_RDMSYSTIME;
		numberOfEntries = 3;
		break;
	case STM_TAG_CNT:
		type = QModbusDataUnit::InputRegisters;
		startAddress = ADDRESS_TAGCOUNT;
		numberOfEntries = 1;
		break;
	case STM_TAG_TEMP:
		type = QModbusDataUnit::InputRegisters;
		startAddress = STARTADDRESS_TEMPERATURE;
		numberOfEntries = m_CurrentTagcnt;
		break;
	case STM_TAG_RSSI:
		type = QModbusDataUnit::InputRegisters;
		startAddress = STARTADDRESS_RSSI;
		numberOfEntries = m_CurrentTagcnt;
		break;
	case STM_TAG_OCRSSI:
		type = QModbusDataUnit::InputRegisters;
		startAddress = STARTADDRESS_OCRSSI;
		numberOfEntries = m_CurrentTagcnt;
		break;
	case STM_TAG_EPC:
		type = QModbusDataUnit::InputRegisters;
		startAddress = STARTADDRESS_EPC;
		numberOfEntries = m_CurrentTagcnt * 8;				//EPC length is 8 WORDS 
		break;
	case STM_TAG_ONLINE:
		type = QModbusDataUnit::DiscreteInputs;
		startAddress = STARTADDRESS_ONLINE;
		numberOfEntries = m_CurrentTagcnt;
		break;
	case STM_TAG_ALARM:
		type = QModbusDataUnit::DiscreteInputs;
		startAddress = STARTADDRESS_ALARM;
		numberOfEntries = m_CurrentTagcnt;
		break;

	case STM_TAG_TEMPLIMIT:
		type = QModbusDataUnit::HoldingRegisters;
		startAddress = STARTADDRESS_TEMPLIMIT;
		numberOfEntries = m_CurrentTagcnt;
		break;		
	}

    return QModbusDataUnit(type, startAddress, numberOfEntries);
}

void iTmsTest::dataHandler(QModbusDataUnit unit)
{
	switch (unit.registerType())
	{
	case QModbusDataUnit::InputRegisters:
	{
		if (unit.startAddress() == STARTADDRESS_RDMSYSTIME)
		{
			int year = HIBYTE(unit.value(0));
			int month = LOBYTE(unit.value(0));
			int day = HIBYTE(unit.value(1));
			int hour = LOBYTE(unit.value(1));
			int minute = HIBYTE(unit.value(2));
			int second = LOBYTE(unit.value(2));

			QString timestr = QString("20%1-%2-%3 %4:%5:%6").arg(year,2,10,QChar('0')).arg(month, 2, 10, QChar('0')).arg(day, 2, 10, QChar('0')).arg(hour, 2, 10, QChar('0')).arg(minute, 2, 10, QChar('0')).arg(second, 2, 10, QChar('0'));
			ui.leRdmTime->setText(timestr);
		}
		else if(unit.startAddress() == STARTADDRESS_RDMINFO)
		{
			QByteArray namebytes;
			namebytes.clear();
			for (int i = 0; i < 8; i++)
			{
				quint16 word = unit.value(i);
				namebytes.append(HIBYTE(word));
				namebytes.append(LOBYTE(word));
			}
			m_RdmName = QString(namebytes);
			ui.leRdmName->setText(m_RdmName);

			QString version =QString("V%1").arg(unit.value(8),3,10,QChar('0'));
			version.insert(2, QChar('.'));
			version.insert(4, QChar('.'));

			ui.leRdmVersion->setText(version);
		}
		else if ((unit.startAddress() == ADDRESS_TAGCOUNT) && (unit.valueCount() == 1))
		{
			m_CurrentTagcnt = unit.value(0);
			if (tagModel->rowCount() == 0)
			{
				insertNewTag2DB(m_CurrentTagcnt);
			}
		}
		else if (unit.startAddress() == STARTADDRESS_TEMPERATURE)								//temperature
		{
			for (uint i = 0; i < unit.valueCount(); i++)
			{
				float temp = unit.value(i)*0.1f;

				QSqlRecord record = tagModel->record(i);

				if (!record.isEmpty()) {
					record.setValue("TEMP", temp);
					tagModel->setRecord(i, record);
					tagModel->select();
				}
			}
		}
		else if (unit.startAddress() == STARTADDRESS_RSSI)								//RSSI
		{
			for (uint i = 0; i < unit.valueCount(); i++)
			{
				qint16 rssi = unit.value(i);

				QSqlRecord record = tagModel->record(i);
				if (!record.isEmpty()) {
					record.setValue("RSSI", rssi);
					tagModel->setRecord(i, record);
					tagModel->select();
				}

			}
		}
		else if (unit.startAddress() == STARTADDRESS_OCRSSI)								//OC_RSSI
		{
			for (uint i = 0; i < unit.valueCount(); i++)
			{
				quint16 oc_rssi = unit.value(i);

				QSqlRecord record = tagModel->record(i);
				if (!record.isEmpty()) {
					record.setValue("OCRSSI", oc_rssi);
					tagModel->setRecord(i, record);
					tagModel->select();
				}
			}
		}
		else if (unit.startAddress() == STARTADDRESS_EPC)								//EPC 
		{
			for (uint i = 0; i < unit.valueCount()/8; i++)
			{
				QByteArray epcbytes;

				epcbytes.clear();
				for (int j = 0; j < 8; j++)
				{
					quint16 word = unit.value(8 * i + j);
					epcbytes.append(HIBYTE(word));
					epcbytes.append(LOBYTE(word));
				}
			
				QSqlRecord record = tagModel->record(i);
				if (!record.isEmpty()) {
					record.setValue("EPC", QString(epcbytes));
					tagModel->setRecord(i, record);
					tagModel->select();
				}
			}
		}
	}
		break;

	case QModbusDataUnit::DiscreteInputs:
	{
		if (unit.startAddress() == STARTADDRESS_ONLINE)								//online flag
		{
			for (uint i = 0; i < unit.valueCount(); i++)
			{
				bool online = unit.value(i);

				QSqlRecord record = tagModel->record(i);
				if (!record.isEmpty()) {
					record.setValue("OFFLINE", online);
					tagModel->setRecord(i, record);
					tagModel->select();
				}
			}
		}
		else if (unit.startAddress() == STARTADDRESS_ALARM)							//alarm flag
		{
			for (uint i = 0; i < unit.valueCount(); i++)
			{
				bool alarm = unit.value(i);

				QSqlRecord record = tagModel->record(i);
				if (!record.isEmpty()) {
					record.setValue("ALARM", alarm);
					tagModel->setRecord(i, record);
					tagModel->select();
				}
			}
		}
	}
	break;

	case QModbusDataUnit::HoldingRegisters:
	{
		for (uint i = 0; i < unit.valueCount(); i++)
		{
			quint16 limit = unit.value(i);

			QSqlRecord record = tagModel->record(i);
			if (!record.isEmpty()) {
				record.setValue("TEMPMAX", limit);
				tagModel->setRecord(i, record);
				tagModel->select();
			}
		}
	}

	}
}

void iTmsTest::insertNewTag2DB(int cnt) {

	for (int i = 0; i < cnt; i++) {
		QSqlRecord record = tagModel->record();
		record.setValue("SID", i + 1);
		record.setValue("EPC", " ");						//EPC must not be null
		record.setValue("RDMNAME", m_RdmName);						
		
		tagModel->insertRecord(i, record);
	}
}

void iTmsTest::saveHistorydata()
{
	for (int i = 0; i < tagModel->rowCount(); i++) {
		QSqlRecord record = tagModel->record(i);

		QSqlRecord datarecord = dataModel->record();
		QDateTime c_time = QDateTime::currentDateTime();
		c_time.addMSecs(i);
		datarecord.setValue("TIME", c_time.toString("yy/MM/dd hh:mm:ss.zzz"));
		datarecord.setValue("SID", record.value("SID"));
		datarecord.setValue("EPC", record.value("EPC"));
		datarecord.setValue("TEMP", record.value("TEMP"));
		datarecord.setValue("RSSI", record.value("RSSI"));
		datarecord.setValue("OCRSSI", record.value("OCRSSI"));
		datarecord.setValue("ALARM", record.value("ALARM"));
		datarecord.setValue("OFFLINE", record.value("OFFLINE"));
		datarecord.setValue("RDMNAME", record.value("RDMNAME"));

		dataModel->insertRecord(i, datarecord);
	}
}

void iTmsTest::onSetTempLimit()
{
	//To do:no tag selectd , ignore it
	//if (ui.tableViewOnline->currentIndex().row() < 0) return;

	QModbusDataUnit::RegisterType type = QModbusDataUnit::HoldingRegisters;
	int startAddress = STARTADDRESS_TEMPLIMIT;
	int numberOfEntries = 1;

	quint16 limit = ui.leTempMax->text().trimmed().toInt();
	writeRequest.setRegisterType(type);
	writeRequest.setValue(0, limit);
}
void iTmsTest::modbuswrite()
{
	if (modbus->state() == QModbusDevice::UnconnectedState)
		return;

	if (auto *reply = modbus->sendWriteRequest(writeRequest, ui.leRdmRTUAddr->text().toInt())) {
		if (!reply->isFinished())
			connect(reply, &QModbusReply::finished, this, [this, reply]() {
			if (reply->error() == QModbusDevice::ProtocolError) {
				statusBar()->showMessage(tr("Write response error: %1 (Mobus exception: 0x%2)")
					.arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16),
					5000);
			}			
			else if (reply->error() == QModbusDevice::NoError) {
				statusBar()->showMessage(tr("Set Temperature limit ok!"),5000);
			}
			else {
				statusBar()->showMessage(tr("Write response error: %1 (code: 0x%2)").
					arg(reply->errorString()).arg(reply->error(), -1, 16), 5000);
			}
			reply->deleteLater();
		});
		else
			delete reply; // broadcast replies return immediately
	}
	else {
		statusBar()->showMessage(tr("Write error: ") + modbus->errorString(), 5000);
	}
}

void iTmsTest::OnCurrentChanged()
{
	QModelIndex index= ui.tableViewOnline->currentIndex();

	if (index.row() < 0) return;

	//QSortFilterProxyModel *proxy = static_cast<QSortFilterProxyModel*>(ui.tableViewOnline->model());
		
	QSqlRecord record=tagModel->record(index.row());
	int sid = record.value("SID").toInt();
	qDebug() << "SID=" << sid;
	ui.leTempMax->setText(record.value("TEMPMAX").toString());
	writeRequest.setStartAddress(STARTADDRESS_TEMPLIMIT + sid - 1);
}