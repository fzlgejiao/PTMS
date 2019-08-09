#include "icfgpanel.h"
#include "irdm.h"
#include "Model.h"
#include <QtGui>

iCfgPanel::iCfgPanel(QWidget *parent)
	: QTabWidget(parent), netcmd(EthernetCmd::Instance())
{
	ui.setupUi(this);

	setTabText(0,QString::fromLocal8Bit("通用设置"));
	setTabText(1, QString::fromLocal8Bit("传感器设置"));
	setTabText(2, QString::fromLocal8Bit("IoT设置"));


	model = new TagModel(this);

	//ui.tableTags->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableTags->setSelectionBehavior(QAbstractItemView::SelectRows);
	//ui.tableTags->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableTags->setAlternatingRowColors(true);

	QHeaderView *headerView = ui.tableTags->horizontalHeader();
	headerView->setVisible(true);
	headerView->setStretchLastSection(true);

	ui.tableTags->setModel(model);
	ui.tableTags->hideColumn(_Model::SID);
	ui.tableTags->hideColumn(_Model::TEMP);
	ui.tableTags->hideColumn(_Model::RSSI);
	ui.tableTags->hideColumn(_Model::OCRSSI);

	setCurrentIndex(0);

	connect(&netcmd, SIGNAL(ModbusParamReady(MSG_PKG&)), this, SLOT(OnModbusParameters(MSG_PKG&)));
	connect(&netcmd, SIGNAL(ParaTagsReady(MSG_PKG&)), this, SLOT(OnParaTagsFound(MSG_PKG&)));

	connect(ui.btnRemoveTag, SIGNAL(clicked()), this, SLOT(OnRemoveTag()));
	connect(ui.btnEditTag, SIGNAL(clicked()), this, SLOT(OnEditTag()));

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
	Tags_Parameters tags;
	memset(&tags, 0, sizeof(tags));
	tags.Header.tagcount = 2;
	tags.Tags[0].uid = 12345;
	strcpy(tags.Tags[0].name, "ABCD");
	strcpy(tags.Tags[0].note, "Note1");
	tags.Tags[1].uid = 67890;
	strcpy(tags.Tags[1].name, "EFGH");
	strcpy(tags.Tags[1].note, "Note2");
	MSG_PKG msg;
	memcpy(msg.cmd_pkg.data, &tags, sizeof(tags));
	OnParaTagsFound(msg);


	int row = ui.tableTags->currentIndex().row();
	QModelIndex index = ui.tableTags->model()->index(row, 0, QModelIndex());
	ui.tableTags->edit(index);
}
void iCfgPanel::OnRdmSelected(iRdm *Rdm)
{
	ui.leRdmName->setText(Rdm->m_name);
	ui.leIPAddress->setText(Rdm->m_ip);
}
void iCfgPanel::OnTagAdded(iTag *tag)
{
	setCurrentIndex(1);																				//switch to tags tab
	//todo: check if tag already exists
	if (model->hasTag(tag->uid()))
		return;
	iTag *newTag = new iTag(*tag);
	model->insertRow(0, newTag);
}
void iCfgPanel::OnParaTagsFound(MSG_PKG& msg)
{
	Tags_Parameters *tags = (Tags_Parameters *)msg.cmd_pkg.data;
	for (int i = 0; i < tags->Header.tagcount; i++)
	{
		if (model->hasTag(tags->Tags[i].uid))
			continue;
		iTag *tag = new iTag(tags->Tags[i].uid, tags->Tags[i].name);
		//todo: fill all parameters of tag
		tag->t_note = tags->Tags[i].note;
		model->insertRow(0, tag);
	}
}