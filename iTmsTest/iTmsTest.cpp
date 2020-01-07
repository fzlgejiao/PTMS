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
#include <QLoggingCategory>

#define	ERR_SHOW_TIME		3000

iTmsTest::iTmsTest(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	QLoggingCategory::setFilterRules(QStringLiteral("qt.modbus.warning=true"));
	this->setWindowTitle(QString("PTMS - MODBUS %1").arg(qApp->applicationVersion()));

	modbus = new QModbusRtuSerialMaster(this);

	ui.btnConnect->setEnabled(true);
	ui.btnDisconnect->setEnabled(false);
	ui.leCompiledTime->setText(tr(__DATE__)+","+tr(__TIME__));
	OnRefresh();
	request_cnt = 0;
	reply_cnt = 0;

	paritymap.insert(0, QSerialPort::NoParity);
	paritymap.insert(1, QSerialPort::OddParity);
	paritymap.insert(2, QSerialPort::EvenParity);

	DB_clearTags();

	//for tags
	tagModel = new QSqlTableModel(this);
	tagModel->setTable("TAGS");
	tagModel->select();

	QSortFilterProxyModel* proxyModelTags = new QSortFilterProxyModel();
	proxyModelTags->setSourceModel(tagModel);
	proxyModelTags->setDynamicSortFilter(true);

	ui.tableViewOnline->setSortingEnabled(true);
	ui.tableViewOnline->setModel(proxyModelTags);
	ui.tableViewOnline->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableViewOnline->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableViewOnline->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableViewOnline->setAlternatingRowColors(true);
	
	writeRequest = QModbusDataUnit(QModbusDataUnit::Invalid, 0, 1);

	//for data
	dataModel = new QSqlTableModel(this);
	dataModel->setTable("DATA");
	dataModel->select();

	QSortFilterProxyModel* proxyModelData = new QSortFilterProxyModel();
	proxyModelData->setSourceModel(dataModel);
	proxyModelData->setDynamicSortFilter(true);

	ui.tableViewHistory->setModel(dataModel);
	ui.tableViewHistory->setSortingEnabled(true);
	ui.tableViewHistory->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableViewHistory->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableViewHistory->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableViewHistory->setAlternatingRowColors(true);
	ui.tableViewHistory->setColumnWidth(0, 250);

	ui.tabWidget->setCurrentIndex(0);

	m_nTagStm = 0;
	m_CurrentTagcnt = 0;
	m_nTimerId_200ms = startTimer(200);
	m_nTimerId_1s = startTimer(1000);
	m_nTimerId_5s = startTimer(5000);
	m_nTimerId_5min = startTimer(300000);

	ui.leTempMax->setValidator(new QIntValidator(0, 120, this));

	connect(modbus, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
		statusBar()->showMessage(modbus->errorString(), ERR_SHOW_TIME);
	});
	connect(modbus, &QModbusClient::stateChanged,this, &iTmsTest::OnStateChanged);

	//connect(ui.tableViewOnline->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(OnCurrentChanged()));
	//connect(ui.tableViewHistory->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(OnTagDataSelectChanged(const QModelIndex &, const QModelIndex &)));
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
		modbus->setConnectionParameter(QModbusDevice::SerialPortNameParameter,
			ui.cbxComm->currentText());
		
		modbus->setConnectionParameter(QModbusDevice::SerialParityParameter, paritymap.value(ui.cbxParityBit->currentIndex()));
		modbus->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,
			ui.cbxBaudRate->currentText().toInt());
		modbus->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,
			ui.cbxDataBit->currentText().toInt());
		modbus->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,
			ui.cbxStopBit->currentText().toInt());
			
		modbus->setTimeout(1000);																	//response timeout
		modbus->setNumberOfRetries(0);
		if (!modbus->connectDevice()) {
			statusBar()->showMessage(tr("Connect failed: ") + modbus->errorString(), ERR_SHOW_TIME);
		}
		else {
			ui.btnConnect->setEnabled(false);
			ui.btnDisconnect->setEnabled(true);
		}
	}
}
void iTmsTest::OnDisconnect()
{
	request_cnt = 0;
	reply_cnt = 0;
	modbus->disconnectDevice();
	ui.btnConnect->setEnabled(true);
	ui.btnDisconnect->setEnabled(false);
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
		if( (writeRequest.registerType()!= QModbusDataUnit::Invalid) && (writeRequest.valueCount()>0)){
			modbuswrite();
			writeRequest.setRegisterType(QModbusDataUnit::Invalid);			
		}
		else {
			//time read one kind of tag data
			read();
			m_nTagStm++;
			if (m_nTagStm == STM_TAG_END)
				m_nTagStm = 0;
		}
	}
	else if (event->timerId() == m_nTimerId_1s)
	{
		ui.leRequestCnt->setText(QString::number(request_cnt));
		ui.leReplyCnt->setText(QString::number(reply_cnt));

		if( (request_cnt - reply_cnt) > 20) 
		{
			//reconnect modbus
			qDebug(QString("Reconnect Modbus :Request=%1 Reply=%2").arg(request_cnt).arg(reply_cnt).toStdString().c_str());
			request_cnt = 0;
			reply_cnt = 0;
			modbus->disconnectDevice();
			OnConnect();
		}
		//time to save and refresh tags from table 'TAGS'
		DB_saveTags();																				//save tags
		tagModel->select();
	}
	else if (event->timerId() == m_nTimerId_5s)
	{
		
	}
	else if (event->timerId() == m_nTimerId_5min)
	{
		//time to copy table 'TAGS' into table 'DATA'
		DB_saveHistory();
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
void iTmsTest::OnDeleteData()
{
	QSortFilterProxyModel *proxy = static_cast<QSortFilterProxyModel*>(ui.tableViewHistory->model());
	QItemSelectionModel *selectionModel = ui.tableViewHistory->selectionModel();

	QModelIndexList indexes = selectionModel->selectedRows();
	QModelIndex index;

	foreach(index, indexes) {
		int row = proxy->mapToSource(index).row();
		dataModel->removeRows(row, 1, QModelIndex());
	}
}
void iTmsTest::OnRefreshData()
{
	dataModel->select();
}
void iTmsTest::OnStateChanged(int state)
{

	if (state == QModbusDevice::ConnectedState)
	{
		ui.btnConnect->setEnabled(false);
		ui.btnDisconnect->setEnabled(true);
		statusBar()->showMessage(tr("ModbusDevice::connected"), 0);
	}
	else
	{
		ui.btnConnect->setEnabled(true);
		ui.btnDisconnect->setEnabled(false);
		QString error;
		if (state == QModbusDevice::UnconnectedState)
			error = tr("ModbusDevice::unconnected");
		else if (state == QModbusDevice::ConnectingState)
			error = tr("ModbusDevice::connecting");
		else if (state == QModbusDevice::ClosingState)
			error = tr("ModbusDevice::closing");

		statusBar()->showMessage(error, 0);
	}
}
void iTmsTest::read()
{
	if (modbus->state() == QModbusDevice::ConnectedState)
	{
		//statusBar()->clearMessage();
		QModbusDataUnit request = readRequest();
		if (request.valueCount() == 0) return;
		if (auto *reply = modbus->sendReadRequest(request, ui.leRdmRTUAddr->text().toInt())) {
			if (!reply->isFinished())
			{
				request_cnt++;
				connect(reply, &QModbusReply::finished, this, &iTmsTest::readReady);
			}
			else
				delete reply; // broadcast replies return immediately
		}
		else {
			statusBar()->showMessage(tr("Read error: ") + modbus->errorString(), ERR_SHOW_TIME);
		}
	}
}
void iTmsTest::readReady()
{
	auto reply = qobject_cast<QModbusReply *>(sender());
	if (!reply)
		return;

	if (reply->error() == QModbusDevice::NoError) {
		reply_cnt++;
		const QModbusDataUnit unit = reply->result();
		for (uint i = 0; i < unit.valueCount(); i++) {
			const QString entry = tr("Address: 0x%1, Value: %2\n")
									.arg((unit.startAddress() + i),4,16,QChar('0'))
									.arg(QString::number(unit.value(i),unit.registerType() <= QModbusDataUnit::Coils ? 10 : 16));
			ui.plainTextEdit->appendPlainText(entry);
		}
		dataHandler(unit);
	}
	else if (reply->error() == QModbusDevice::ProtocolError) {
		statusBar()->showMessage(tr("Read response error: %1 (Mobus exception: 0x%2)").
			arg(reply->errorString()).
			arg(reply->rawResult().exceptionCode(), -1, 16), ERR_SHOW_TIME);
	}
	else {
		statusBar()->showMessage(tr("Read response error: %1 (code: 0x%2)").
			arg(reply->errorString()).
			arg(reply->error(), -1, 16), ERR_SHOW_TIME);
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
	QSqlQuery	query;
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
				DB_createTags(m_CurrentTagcnt);
			}
		}
		else if (unit.startAddress() == STARTADDRESS_TEMPERATURE)								//temperature
		{
			for (uint i = 0; i < unit.valueCount(); i++)
			{
				iTag* tag = getTag(i + 1);
				if(tag)
					tag->T_temp = (qint16)(unit.value(i))*0.1f;
			}
		}
		else if (unit.startAddress() == STARTADDRESS_RSSI)								//RSSI
		{
			for (uint i = 0; i < unit.valueCount(); i++)
			{
				iTag* tag = getTag(i + 1);
				if (tag)
					tag->T_rssi = unit.value(i);
			}

		}
		else if (unit.startAddress() == STARTADDRESS_OCRSSI)								//OC_RSSI
		{
			for (uint i = 0; i < unit.valueCount(); i++)
			{
				iTag* tag = getTag(i + 1);
				if (tag)
					tag->T_OC_rssi = unit.value(i);;
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

				iTag* tag = getTag(i + 1);
				if (tag)
					tag->T_epc = QString(epcbytes);
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
				iTag* tag = getTag(i + 1);
				if (tag)
					tag->T_online = unit.value(i);
			}
		}
		else if (unit.startAddress() == STARTADDRESS_ALARM)							//alarm flag
		{
			for (uint i = 0; i < unit.valueCount(); i++)
			{
				iTag* tag = getTag(i + 1);
				if (tag)
					tag->T_alarm = unit.value(i);
			}
		}
	}
	break;

	case QModbusDataUnit::HoldingRegisters:
	{
		for (uint i = 0; i < unit.valueCount(); i++)
		{
			iTag* tag = getTag(i + 1);
			if (tag)
				tag->T_uplimit = unit.value(i);
		}
	}
	break;
	}
}

void iTmsTest::DB_createTags(int cnt)
{
	qDeleteAll(listTags);
	listTags.clear();

	ui.combo_sid->clear();
	for (int i = 0; i < cnt; i++) {
		QSqlRecord record = tagModel->record();
		record.setValue("SID", i + 1);
		record.setValue("EPC", " ");						//EPC must not be null
		//record.setValue("RDMNAME", m_RdmName);

		
		if (tagModel->insertRecord(i, record) == true)
		{
			ui.combo_sid->addItem(QString::number(i + 1));
			iTag* tag = new iTag(i + 1);
			if(tag)
				listTags.insert(i + 1, tag);
		}
	}
	if (cnt > 0)
		ui.combo_sid->setCurrentIndex(0);
}
void iTmsTest::DB_saveTags()
{
	if (modbus->state() != QModbusDevice::ConnectedState)
		return;

	QSqlQuery	query;
	bool ret;
	for (iTag* tag : listTags)
	{
		ret = query.exec(QString("UPDATE TAGS SET EPC = '%1',TEMP = '%2',RSSI = '%3',OCRSSI = '%4',TEMPMAX = %5,ALARM = %6 WHERE SID = %7")
			.arg(tag->T_epc)
			.arg(tag->Temp())
			.arg(tag->RSSI())
			.arg(tag->OCRSSI())
			.arg(tag->T_uplimit)
			.arg(tag->T_alarm)
			.arg(tag->T_sid));
	}
}
void iTmsTest::DB_saveHistory()
{
	if (modbus->state() != QModbusDevice::ConnectedState)
		return;

	//QDateTime c_time = QDateTime::currentDateTime();
	//int i = 0;
	//for (iTag* tag : listTags)
	//{
	//	QSqlRecord datarecord = dataModel->record();
	//	c_time = c_time.addMSecs(i++);
	//	datarecord.setValue("TIME", c_time.toString("yyyy/MM/dd hh:mm:ss.zzz"));
	//	datarecord.setValue("SID", tag->T_sid);
	//	datarecord.setValue("EPC", tag->T_epc);
	//	datarecord.setValue("TEMP", tag->Temp());
	//	datarecord.setValue("RSSI", tag->RSSI());
	//	datarecord.setValue("OCRSSI", tag->OCRSSI());
	//	datarecord.setValue("TEMPMAX", tag->T_uplimit);
	//	datarecord.setValue("ALARM", tag->T_alarm);
	//	//datarecord.setValue("OFFLINE", record.value("OFFLINE"));
	//	//datarecord.setValue("RDMNAME", record.value("RDMNAME"));
	//	dataModel->insertRecord(0, datarecord);
	//}

	QSqlQuery	query;
	bool ret;
	int i = 0;
	for (iTag* tag : listTags)
	{
		QDateTime c_time = QDateTime::currentDateTime();
		c_time.addMSecs(i++);
		ret = query.exec(QString("INSERT INTO DATA VALUES('%1',%2,'%3','%4','%5','%6', %7,%8)")
			.arg(c_time.toString("yyyy/MM/dd hh:mm:ss.zzz"))
			.arg(tag->T_sid)
			.arg(tag->T_epc)
			.arg(tag->T_online ? QString::number(tag->T_temp, 'f', 1) : "--.-")
			.arg(tag->T_online ? QString::number(tag->T_rssi) : "---")
			.arg(tag->T_online ? QString::number(tag->T_OC_rssi) : "---")
			.arg(tag->T_uplimit)
			.arg(tag->T_alarm));
	}
}

void iTmsTest::onSetTempLimit()
{
	if((ui.combo_sid->currentText().isEmpty()) || (ui.leTempMax->text().isEmpty()) ) return;
	
	int startAddress = STARTADDRESS_TEMPLIMIT + ui.combo_sid->currentText().toInt() - 1;

	quint16 limit = ui.leTempMax->text().trimmed().toInt();
	writeRequest.setRegisterType(QModbusDataUnit::HoldingRegisters);
	writeRequest.setStartAddress(startAddress);
	writeRequest.setValue(0, limit);
}
void iTmsTest::modbuswrite()
{
	if (modbus->state() == QModbusDevice::UnconnectedState)
		return;

	if (auto *reply = modbus->sendWriteRequest(writeRequest, ui.leRdmRTUAddr->text().toInt())) {
		if (!reply->isFinished())
		{
			request_cnt++;
			connect(reply, &QModbusReply::finished, this, [this, reply]() {
				if (reply->error() == QModbusDevice::ProtocolError) {
					statusBar()->showMessage(tr("Write response error: %1 (Mobus exception: 0x%2)")
						.arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16),
						ERR_SHOW_TIME);
				}
				else if (reply->error() == QModbusDevice::NoError) {
					reply_cnt++;
					statusBar()->showMessage(tr("Set Temperature limit ok!"), ERR_SHOW_TIME);
				}
				else {
					statusBar()->showMessage(tr("Write response error: %1 (code: 0x%2)").
						arg(reply->errorString()).arg(reply->error(), -1, 16), ERR_SHOW_TIME);
				}
				reply->deleteLater();
			});
		}
		else
			delete reply; // broadcast replies return immediately
	}
	else {
		statusBar()->showMessage(tr("Write error: ") + modbus->errorString(), ERR_SHOW_TIME);
	}
}

void iTmsTest::OnCurrentChanged()
{
	QModelIndex index= ui.tableViewOnline->currentIndex();

	if (index.row() < 0) 
		return;

	//QSortFilterProxyModel *proxy = static_cast<QSortFilterProxyModel*>(ui.tableViewOnline->model());
		
	QSqlRecord record=tagModel->record(index.row());
	int sid = record.value("SID").toInt();
	qDebug() << "SID=" << sid;
	ui.leTempMax->setText(record.value("TEMPMAX").toString());
	writeRequest.setStartAddress(STARTADDRESS_TEMPLIMIT + sid - 1);
}
void iTmsTest::OnTagDataSelectChanged(const QModelIndex &current, const QModelIndex &previous)
{
	if (current.isValid())
	{
		//ui.btnDelete->setEnabled(true);

		//QSortFilterProxyModel *proxy = static_cast<QSortFilterProxyModel*>(ui.tableViewHistory->model());
		//QItemSelectionModel *selectionModel = ui.tableViewHistory->selectionModel();

		//QModelIndexList indexes = selectionModel->selectedRows();
		//QModelIndex index;

		////foreach(index, indexes) 
		//{
		//	int row = proxy->mapToSource(current).row();

		//	QSqlRecord record = dataModel->record(row);
		//	int sid = record.value("SID").toInt();
		//	qDebug() << "SID=" << sid;
		//	ui.leTempMax->setText(record.value("TEMPMAX").toString());
		//	ui.leRdmVersion->setText(record.value("EPC").toString());

		//}

	}
	else
		ui.btnDelete->setEnabled(false);
}