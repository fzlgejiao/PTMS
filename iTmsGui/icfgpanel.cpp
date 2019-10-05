#include "icfgpanel.h"
#include "irdm.h"
#include "Model.h"
#include <QtGui>
#include <QMessageBox>
#include <QFile>
#include <QtXml>
#include <iostream>

bool TagAscendingbyEpc(iTag *tag1, iTag *tag2)
{
	return (tag1->epc() < tag2->epc());
}

iCfgPanel::iCfgPanel(QWidget *parent)
	: QTabWidget(parent)
	, oSys(iSys::Instance())
	, netcmd(EthernetCmd::Instance())
{
	ui.setupUi(this);

	setStyleSheet("QTableView::item{selection-color: white; selection-background-color: rgb(20, 20, 125);} QLineEdit:hover{border: 2px solid blue; }");

	paritymap.insert(0, QSerialPort::NoParity);
	paritymap.insert(1, QSerialPort::EvenParity);
	paritymap.insert(2, QSerialPort::OddParity);

	filename = "";

	setTabText(0,QString::fromLocal8Bit("通用设置"));
	setTabText(1, QString::fromLocal8Bit("传感器设置"));
	setTabText(2, QString::fromLocal8Bit("IoT设置"));
		
	QRegExp regExp("^[^\u4e00-\u9fa5]{0,}$");	
	
	ui.lePrdKey->setValidator(new QRegExpValidator(regExp, parent));
	ui.lePrdKey->setToolTip(QString::fromLocal8Bit("请输入字母,字符和数字"));
	
	ui.leDeviceName->setValidator(new QRegExpValidator(regExp, parent));
	ui.leDeviceName->setToolTip(QString::fromLocal8Bit("请输入字母,字符和数字"));

	ui.leDeviceSecret->setValidator(new QRegExpValidator(regExp, parent));
	ui.leDeviceSecret->setToolTip(QString::fromLocal8Bit("请输入字母,字符和数字"));

	ui.leRegion->setValidator(new QRegExpValidator(regExp, parent));
	ui.leRegion->setToolTip(QString::fromLocal8Bit("请输入字母,字符和数字"));
	
	tagModel = new TagModel(this);
	tagModel->setEditColumns((1 << _Model::UPLIMIT) | (1 << _Model::NOTE));
	oSys.tagModelPara = tagModel;

	ui.tableTags->setEditTriggers(QAbstractItemView::DoubleClicked);
	ui.tableTags->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableTags->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableTags->setAlternatingRowColors(true);

	QHeaderView *headerView = ui.tableTags->horizontalHeader();
	headerView->setVisible(true);
	headerView->setStretchLastSection(true);

	ui.tableTags->setModel(tagModel);
	ui.tableTags->hideColumn(_Model::SID);
	ui.tableTags->hideColumn(_Model::TEMP);
	ui.tableTags->hideColumn(_Model::ALARM);
	ui.tableTags->hideColumn(_Model::RSSI);
	ui.tableTags->hideColumn(_Model::OCRSSI);
	ui.tableTags->setColumnWidth(_Model::UID, 180);

	ui.btnEditTagLimit->setEnabled(false);
	ui.btnEditTagNote->setEnabled(false);
	ui.btnRemoveTag->setEnabled(false);

	ui.tableTags->setItemDelegateForColumn(_Model::UPLIMIT, new RangeLimitDelegate(1, 120, this));
	ui.tableTags->setItemDelegateForColumn(_Model::NOTE, new LengthLimitDelegate(TAG_NOTE_SIZE,true, this));

	setCurrentIndex(0);

	connect(&netcmd, SIGNAL(ModbusParamReady(MSG_PKG&)), this, SLOT(OnMsgModbusParameters(MSG_PKG&)));
	connect(&netcmd, SIGNAL(TagsParaReady(MSG_PKG&)), this, SLOT(OnMsgTagsParaReady(MSG_PKG&)));
	connect(&netcmd, SIGNAL(TagEpcReady(MSG_PKG&)), this, SLOT(OnMsgTagEpc(MSG_PKG&)));
	connect(&netcmd, SIGNAL(IotParaReady(MSG_PKG&)), this, SLOT(OnMsgIoTParameters(MSG_PKG&)));

	connect(ui.btnRemoveTag, SIGNAL(clicked()), this, SLOT(OnRemoveTag()));
	connect(ui.btnEditTagLimit, SIGNAL(clicked()), this, SLOT(OnEditTagLimit()));
	connect(ui.btnEditTagNote, SIGNAL(clicked()), this, SLOT(OnEditTagNote()));
	connect(ui.tableTags->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(OnTagSelectChanged(const QModelIndex &)));

	//UI change
	connect(ui.leRdmName, SIGNAL(textEdited(const QString &)), this, SLOT(OnRdmModified()));
	connect(ui.leRdmNote, SIGNAL(textEdited(const QString &)), this, SLOT(OnRdmModified()));
	connect(ui.leMbRtuAddr, SIGNAL(textEdited(const QString &)), this, SLOT(OnRdmModified()));
	connect(ui.lePrdKey, SIGNAL(textEdited(const QString &)), this, SLOT(OnRdmModified()));
	connect(ui.leDeviceName, SIGNAL(textEdited(const QString &)), this, SLOT(OnRdmModified()));
	connect(ui.leDeviceSecret, SIGNAL(textEdited(const QString &)), this, SLOT(OnRdmModified()));
	connect(ui.leRegion, SIGNAL(textEdited(const QString &)), this, SLOT(OnRdmModified()));
	connect(ui.cbxMbType, SIGNAL(currentIndexChanged(int )), this, SLOT(OnRdmModified()));
	connect(ui.cbxMbBaurate, SIGNAL(currentIndexChanged(int )), this, SLOT(OnRdmModified()));
	connect(ui.cbxMbParity, SIGNAL(currentIndexChanged(int )), this, SLOT(OnRdmModified()));

	connect(tagModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(OnRdmModified()));

	connect(ui.leRdmName, &QLineEdit::textChanged, this, &iCfgPanel::Ontextchanged);
	connect(ui.leRdmNote, &QLineEdit::textChanged, this, &iCfgPanel::Ontextchanged);
}

iCfgPanel::~iCfgPanel()
{
}
void iCfgPanel::OnMsgModbusParameters(MSG_PKG& msg)
{
	disconnect(ui.cbxMbType, SIGNAL(currentIndexChanged(int )), this, SLOT(OnRdmModified()));
	disconnect(ui.cbxMbBaurate, SIGNAL(currentIndexChanged(int )), this, SLOT(OnRdmModified()));
	disconnect(ui.cbxMbParity, SIGNAL(currentIndexChanged(int )), this, SLOT(OnRdmModified()));
	MODBUS_Paramters* modbus = (MODBUS_Paramters *)msg.cmd_pkg.data;
	ui.cbxMbType->setCurrentIndex(modbus->type);
	ui.leMbRtuAddr->setText(QString::number(modbus->rtu_address));
	ui.cbxMbBaurate->setCurrentIndex(ui.cbxMbBaurate->findText(QString::number(modbus->rtu_baudrate)));
	ui.leMbTcpPort->setText(QString::number(modbus->tcp_port));
	ui.leMbComPort->setText(modbus->rtu_comname);
	ui.cbxMbParity->setCurrentIndex(paritymap.key((QSerialPort::Parity)modbus->rtu_parity));
	connect(ui.cbxMbType, SIGNAL(currentIndexChanged(int )), this, SLOT(OnRdmModified()));
	connect(ui.cbxMbBaurate, SIGNAL(currentIndexChanged(int )), this, SLOT(OnRdmModified()));
	connect(ui.cbxMbParity, SIGNAL(currentIndexChanged(int )), this, SLOT(OnRdmModified()));
}
void iCfgPanel::OnMsgIoTParameters(MSG_PKG& msg)
{
	IOT_Paramters* iot = (IOT_Paramters *)msg.cmd_pkg.data;

	ui.lePrdKey->setText(iot->productkey);
	ui.leDeviceName->setText(iot->devicename);
	ui.leDeviceSecret->setText(iot->devicesecret);
	ui.leRegion->setText(iot->regionid);
}
void iCfgPanel::OnRemoveTag()
{
	//int row = ui.tableTags->currentIndex().row();
	//if(row != -1)
	//	tagModel->removeRows(row, 1);

	QItemSelectionModel *selectionModel = ui.tableTags->selectionModel();
	QModelIndexList indexes = selectionModel->selectedRows();
	for(QModelIndex index : indexes) {
		int row = index.row();
		if (row != -1)
		{
			tagModel->removeRows(row, 1, QModelIndex());
			OnRdmModified();																		//rdm changed due to tag removed
		}
	}
}
void iCfgPanel::OnEditTagLimit()
{
	int row = ui.tableTags->currentIndex().row();
	QModelIndex index = ui.tableTags->model()->index(row, _Model::UPLIMIT, QModelIndex());
	ui.tableTags->edit(index);
}
void iCfgPanel::OnEditTagNote()
{

	int row = ui.tableTags->currentIndex().row();
	QModelIndex index = ui.tableTags->model()->index(row, _Model::NOTE, QModelIndex());
	ui.tableTags->edit(index);
}
void iCfgPanel::OnRdmSelected(iRdm *rdm)
{
	//clear editing tags when rdm change selected
	if (tagModel->rowCount() > 0)
		tagModel->removeRows(0, tagModel->rowCount());	
	
	if (rdm)
	{
		ui.leRdmName->setText(rdm->m_name);
		ui.leIPAddress->setText(rdm->m_ip);
		ui.leRdmNote->setText(rdm->m_note);
		ui.leComPort->setText(rdm->m_comname);
	}
	else
	{
		ui.leRdmName->setText("");
		ui.leIPAddress->setText("");
		ui.leRdmNote->setText("");
		ui.leComPort->setText("");

		ui.cbxMbType->setCurrentIndex(0);
		ui.leMbRtuAddr->setText("");
		ui.leMbComPort->setText("");
		ui.cbxMbBaurate->setCurrentIndex(0);
		ui.cbxMbParity->setCurrentIndex(0);
		ui.leMbTcpPort->setText("");

		ui.lePrdKey->setText("");
		ui.leDeviceName->setText("");
		ui.leDeviceSecret->setText("");
		ui.leRegion->setText("");
	}

}
void iCfgPanel::OnRdmSaved(iRdm *Rdm)
{
	//todo: save settings to rdm xml file
	//saveRdmXml(Rdm);
}
void iCfgPanel::OnRdmDownloaded(iRdm *Rdm)
{
	//todo: save settings to rdm xml file and download to rdm
	if (!saveRdmXml(Rdm))
	{
		QMessageBox mbx(QMessageBox::Warning, "PTMS", QString::fromLocal8Bit("保存Xml配置文件失败."), QMessageBox::Ok);
		mbx.setMinimumSize(600, 400);
		mbx.exec();
		return;
	}
	netcmd.UDP_fileinfo(Rdm, filename, XmlFile);
	Rdm->setModified(false);
}
void iCfgPanel::OnRdmModified()
{
	emit RdmModified();
}
void iCfgPanel::OnTagSelectChanged(const QModelIndex &index)
{
	if (index.isValid())
	{
		ui.btnEditTagLimit->setEnabled(true);
		ui.btnEditTagNote->setEnabled(true);
		ui.btnRemoveTag->setEnabled(true);
	}
	else
	{
		ui.btnEditTagLimit->setEnabled(false);
		ui.btnEditTagNote->setEnabled(false);
		ui.btnRemoveTag->setEnabled(false);
	}
}
void iCfgPanel::OnTagAdded(iTag *tag)
{
	setCurrentIndex(1);																				//switch to tags tab
	//todo: check if tag already exists
	if (tagModel->rowCount() >= TAG_NUM || tagModel->hasTag(tag->uid()) || tagModel->hasTag(tag->epc()) )
	{
		QMessageBox mbx(QMessageBox::Warning,"PTMS", QString::fromLocal8Bit("添加失败：相同识别号/名称的标签已经存在，或者标签数量已达上限."),QMessageBox::Ok);
		mbx.setMinimumSize(600, 400);
		mbx.exec();
		return;
	}
	tagModel->insertRow(0, new iTag(*tag));															//create a new tag in config panel tag list
	OnRdmModified();																				//rdm changed due to new tag added
}
void iCfgPanel::OnMsgTagsParaReady(MSG_PKG& msg)
{
	if (tagModel->rowCount() > 0)
		tagModel->removeRows(0, tagModel->rowCount());

	Tags_Parameters *tags = (Tags_Parameters *)msg.cmd_pkg.data;
	for (int i = 0; i < tags->Header.tagcount; i++)
	{
		if (tagModel->hasTag(tags->Tags[i].uid, tags->Tags[i].name))									//make sure no duplicated tags 
			continue;
		iTag *tag = new iTag(tags->Tags[i].uid, QString::fromLocal8Bit(tags->Tags[i].name));
		//todo: fill all parameters of tag
		tag->t_sid		= tags->Tags[i].sid;
		tag->t_note		= QString::fromLocal8Bit(tags->Tags[i].note);
		tag->t_uplimit	= tags->Tags[i].upperlimit;
		tagModel->insertRow(0, tag);
	}
}
void iCfgPanel::OnMsgTagEpc(MSG_PKG& msg)
{
	Tag_epc *tagEpc = (Tag_epc *)msg.cmd_pkg.data;

	tagModel->setTagEpc(tagEpc->uid, QString::fromLocal8Bit(tagEpc->epc));								//acked for epc change
}
bool iCfgPanel::saveRdmXml(iRdm *Rdm)
{
	QString mac = Rdm->m_MAC;
	QString dirpath = QCoreApplication::applicationDirPath() + "/RdmXmls/";
	filename = dirpath + QString("iTmsRdm-%1.xml").arg(mac.remove(':'));
	if (!QDir(dirpath).exists())
	{
		if (!QDir().mkdir(dirpath))
			return false;
	}
	QFile file(filename);
	if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate)) {
		std::cerr << "Error: Cannot write file "
			<< qPrintable(file.errorString()) << std::endl;
		return false;
	}

	QXmlStreamWriter xmlWriter(&file);
	xmlWriter.setAutoFormatting(true);
	xmlWriter.writeStartDocument();

	xmlWriter.writeStartElement("rdm");
	xmlWriter.writeAttribute("name", ui.leRdmName->text().trimmed());
	xmlWriter.writeAttribute("org", "herong");
	xmlWriter.writeAttribute("note", ui.leRdmNote->text().trimmed());
	xmlWriter.writeAttribute("ip", Rdm->m_ip);
	xmlWriter.writeAttribute("mac", Rdm->m_MAC);
	
	//******************Configs------start************/
	xmlWriter.writeStartElement("cfg");

	xmlWriter.writeTextElement("com", Rdm->m_comname);

	xmlWriter.writeStartElement("iot");
	xmlWriter.writeAttribute("productkey", ui.lePrdKey->text().trimmed());
	xmlWriter.writeAttribute("devicename", ui.leDeviceName->text().trimmed());
	xmlWriter.writeAttribute("devicesecret", ui.leDeviceSecret->text().trimmed());
	xmlWriter.writeAttribute("regionid", ui.leRegion->text().trimmed());
	xmlWriter.writeCharacters("AliIoT");
	xmlWriter.writeEndElement();					//match "iot" element

	xmlWriter.writeStartElement("gui");
	xmlWriter.writeAttribute("ip", "192.168.0.2");
	xmlWriter.writeAttribute("port", "2871");
	xmlWriter.writeAttribute("version", "v0.0.1");
	xmlWriter.writeEndElement();					//match "gui" element

	xmlWriter.writeStartElement("modbus");
	xmlWriter.writeAttribute("type", ui.cbxMbType->currentText());
	xmlWriter.writeAttribute("comname", ui.leMbComPort->text());
	xmlWriter.writeAttribute("slaveaddress", ui.leMbRtuAddr->text().trimmed());
	xmlWriter.writeAttribute("baudrate", ui.cbxMbBaurate->currentText());
	xmlWriter.writeAttribute("parity", QString::number(paritymap.value(ui.cbxMbParity->currentIndex())));
	xmlWriter.writeAttribute("tcpport", ui.leMbTcpPort->text().trimmed());
	xmlWriter.writeCharacters("");
	xmlWriter.writeEndElement();				//match "modbus" element
	
	xmlWriter.writeEndElement();			//Match "cfg" element

	//******************Tags------start************/
	xmlWriter.writeStartElement("tags");
	int sid = 1;
	//Fixed to sort by epc when save to xml
	qSort(tagModel->taglist().begin(), tagModel->taglist().end(), TagAscendingbyEpc);
	foreach(iTag *tag, tagModel->taglist())
	{
		tag->t_sid = sid;
		xmlWriter.writeStartElement("tag");
		xmlWriter.writeAttribute("sid", QString::number(tag->t_sid));
		xmlWriter.writeAttribute("uid", QString::number(tag->t_uid));
		xmlWriter.writeAttribute("epc", tag->t_epc);
		xmlWriter.writeAttribute("max", QString::number(tag->t_uplimit));
		xmlWriter.writeCharacters(tag->t_note);
		xmlWriter.writeEndElement();
		sid++;
	}
	xmlWriter.writeEndElement();
	//******************Tags------end************/

	xmlWriter.writeEndElement();								//match root element("rdm");

	xmlWriter.writeEndDocument();
	file.close();
	if (file.error()) {
		std::cerr << "Error: Cannot write file "
			<< qPrintable(file.errorString()) << std::endl;
		return false;
	}
	return true;
}

void iCfgPanel::Ontextchanged(QString text)															//for rdm name and note
{
	QLineEdit *edit = qobject_cast<QLineEdit*>(sender());

	int bytes = text.toLocal8Bit().length();
	if (bytes > RDM_NAME_NOTE)
	{
		while (text.toLocal8Bit().length() > RDM_NAME_NOTE)
			text.chop(1);

		edit->setText(text);
	}
}
