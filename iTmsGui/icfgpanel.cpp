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
	: QTabWidget(parent), netcmd(EthernetCmd::Instance())
{
	ui.setupUi(this);

	paritymap.insert(0, QSerialPort::NoParity);
	paritymap.insert(1, QSerialPort::EvenParity);
	paritymap.insert(2, QSerialPort::OddParity);

	filename = "";

	setTabText(0,QString::fromLocal8Bit("通用设置"));
	setTabText(1, QString::fromLocal8Bit("传感器设置"));
	setTabText(2, QString::fromLocal8Bit("IoT设置"));


	model = new TagModel(this);
	model->setEditColumns((1 << _Model::UPLIMIT) | (1 << _Model::NOTE));

	ui.tableTags->setEditTriggers(QAbstractItemView::DoubleClicked);
	ui.tableTags->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableTags->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableTags->setAlternatingRowColors(true);

	QHeaderView *headerView = ui.tableTags->horizontalHeader();
	headerView->setVisible(true);
	headerView->setStretchLastSection(true);

	ui.tableTags->setModel(model);
	ui.tableTags->hideColumn(_Model::SID);
	ui.tableTags->hideColumn(_Model::TEMP);
	ui.tableTags->hideColumn(_Model::ALARM);
	ui.tableTags->hideColumn(_Model::RSSI);
	ui.tableTags->hideColumn(_Model::OCRSSI);
	ui.tableTags->setColumnWidth(_Model::UID, 180);

	ui.btnEditTagLimit->setEnabled(false);
	ui.btnEditTagNote->setEnabled(false);
	ui.btnRemoveTag->setEnabled(false);

	setCurrentIndex(0);

	connect(&netcmd, SIGNAL(ModbusParamReady(MSG_PKG&)), this, SLOT(OnModbusParameters(MSG_PKG&)));
	connect(&netcmd, SIGNAL(TagsParaReady(MSG_PKG&)), this, SLOT(OnTagsParaReady(MSG_PKG&)));
	connect(&netcmd, SIGNAL(TagEpcReady(MSG_PKG&)), this, SLOT(OnTagEpc(MSG_PKG&)));
	connect(&netcmd, SIGNAL(IotParaReady(MSG_PKG&)), this, SLOT(OnIoTParameters(MSG_PKG&)));

	connect(ui.btnRemoveTag, SIGNAL(clicked()), this, SLOT(OnRemoveTag()));
	connect(ui.btnEditTagLimit, SIGNAL(clicked()), this, SLOT(OnEditTagLimit()));
	connect(ui.btnEditTagNote, SIGNAL(clicked()), this, SLOT(OnEditTagNote()));
	connect(ui.tableTags->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(OnTagSelectChanged(const QModelIndex &)));

}

iCfgPanel::~iCfgPanel()
{
}
void iCfgPanel::OnModbusParameters(MSG_PKG& msg)
{
	MODBUS_Paramters* modbus = (MODBUS_Paramters *)msg.cmd_pkg.data;
	ui.cbxMbType->setCurrentIndex(modbus->type);
	ui.leMbRtuAddr->setText(QString::number(modbus->rtu_address));
	ui.cbxMbBaurate->setCurrentIndex(ui.cbxMbBaurate->findText(QString::number(modbus->rtu_baudrate)));
	ui.leMbTcpPort->setText(QString::number(modbus->tcp_port));
	ui.leMbComPort->setText(modbus->rtu_comname);
	ui.cbxMbParity->setCurrentIndex(paritymap.key((QSerialPort::Parity)modbus->rtu_parity));
}
void iCfgPanel::OnIoTParameters(MSG_PKG& msg)
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
	//	model->removeRows(row, 1);

	QItemSelectionModel *selectionModel = ui.tableTags->selectionModel();
	QModelIndexList indexes = selectionModel->selectedRows();
	for(QModelIndex index : indexes) {
		int row = index.row();
		if(row != -1)
			model->removeRows(row, 1, QModelIndex());
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
	if (model->rowCount() > 0)
		model->removeRows(0, model->rowCount());	
	
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
	if (model->hasTag(tag->uid()))
	{
		QMessageBox mbx(QMessageBox::Warning,"PTMS", QString::fromLocal8Bit("标签已经存在."),QMessageBox::Ok);
		mbx.setMinimumSize(600, 400);
		mbx.exec();
		return;
	}
	iTag *newTag = new iTag(*tag);
	model->insertRow(0, newTag);
}
void iCfgPanel::OnTagsParaReady(MSG_PKG& msg)
{
	Tags_Parameters *tags = (Tags_Parameters *)msg.cmd_pkg.data;
	for (int i = 0; i < tags->Header.tagcount; i++)
	{
		if (model->hasTag(tags->Tags[i].uid))														//make sure no duplicated tags 
			continue;
		iTag *tag = new iTag(tags->Tags[i].uid, tags->Tags[i].name);
		//todo: fill all parameters of tag
		tag->t_note = tags->Tags[i].note;
		model->insertRow(0, tag);
	}
}
void iCfgPanel::OnTagEpc(MSG_PKG& msg)
{
	Tag_epc *tagEpc = (Tag_epc *)msg.cmd_pkg.data;

	model->setTagEpc(tagEpc->uid, tagEpc->epc);
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
	xmlWriter.writeAttribute("mac", Rdm->m_MAC);
	xmlWriter.writeAttribute("name", Rdm->m_name);
	xmlWriter.writeAttribute("org", "herong");
	xmlWriter.writeAttribute("note", Rdm->m_note);
	xmlWriter.writeAttribute("ip", Rdm->m_ip);
	
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
	qSort(model->taglist().begin(), model->taglist().end(), TagAscendingbyEpc);
	foreach(iTag *tag, model->taglist())
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
