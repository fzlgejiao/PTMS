#include "icfgpanel.h"
#include "irdm.h"
#include "Model.h"
#include <QtGui>
#include <QMessageBox>

iCfgPanel::iCfgPanel(QWidget *parent)
	: QTabWidget(parent), netcmd(EthernetCmd::Instance())
{
	ui.setupUi(this);

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

	ui.btnEditTag->setEnabled(false);
	ui.btnRemoveTag->setEnabled(false);

	setCurrentIndex(0);

	connect(&netcmd, SIGNAL(ModbusParamReady(MSG_PKG&)), this, SLOT(OnModbusParameters(MSG_PKG&)));
	connect(&netcmd, SIGNAL(TagsParaReady(MSG_PKG&)), this, SLOT(OnTagsParaReady(MSG_PKG&)));

	connect(ui.btnRemoveTag, SIGNAL(clicked()), this, SLOT(OnRemoveTag()));
	connect(ui.btnEditTag, SIGNAL(clicked()), this, SLOT(OnEditTag()));
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
	ui.cbxMbParity->setCurrentIndex(modbus->rtu_parity);
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
void iCfgPanel::OnEditTag()
{
	//todo:test code
	//Tags_Parameters tags;
	//memset(&tags, 0, sizeof(tags));
	//tags.Header.tagcount = 2;
	//tags.Tags[0].uid = 12345;
	//strcpy(tags.Tags[0].name, "ABCD");
	//strcpy(tags.Tags[0].note, "Note1");
	//tags.Tags[1].uid = 67890;
	//strcpy(tags.Tags[1].name, "EFGH");
	//strcpy(tags.Tags[1].note, "Note2");
	//MSG_PKG msg;
	//memcpy(msg.cmd_pkg.data, &tags, sizeof(tags));
	//OnTagsParaReady(msg);


	int row = ui.tableTags->currentIndex().row();
	QModelIndex index = ui.tableTags->model()->index(row, _Model::UPLIMIT, QModelIndex());
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
	}
	else
	{
		ui.leRdmName->setText("");
		ui.leIPAddress->setText("");
		ui.leRdmNote->setText("");
	}

}
void iCfgPanel::OnRdmSaved(iRdm *Rdm)
{
	//todo: save settings to rdm xml file
}
void iCfgPanel::OnRdmDownloaded(iRdm *Rdm)
{
	//todo: save settings to rdm xml file and download to rdm
}

void iCfgPanel::OnTagSelectChanged(const QModelIndex &index)
{
	if (index.isValid())
	{
		ui.btnEditTag->setEnabled(true);
		ui.btnRemoveTag->setEnabled(true);
	}
	else
	{
		ui.btnEditTag->setEnabled(false);
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