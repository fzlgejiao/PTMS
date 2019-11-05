#include "idataview.h"
#include "irdm.h"
#include "Model.h"
#include <QtGui>

iDataView::iDataView(QWidget *parent)
	: QTabWidget(parent)
	, oSys(iSys::Instance())
	, netcmd(EthernetCmd::Instance())
{
	ui.setupUi(this);

	ui.tableCurrentTags->setStyleSheet("QTableView::item{selection-color: white; selection-background-color: rgb(20, 20, 125);}");

	tagModel = new TagModel(this);
	oSys.tagModelData = tagModel;

	ui.tableCurrentTags->setModel(tagModel);
	ui.tableCurrentTags->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableCurrentTags->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableCurrentTags->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableCurrentTags->setAlternatingRowColors(true);

	QHeaderView *headerView = ui.tableCurrentTags->horizontalHeader();
	headerView->setStretchLastSection(true);

	ui.tableCurrentTags->hideColumn(_Model::UPLIMIT);
	ui.tableCurrentTags->setColumnWidth(_Model::SID, 70);
	ui.tableCurrentTags->setColumnWidth(_Model::UID, 180);

	connect(&netcmd, SIGNAL(TagsDataReady(MSG_PKG&)), this, SLOT(OnMsgTagsDataReady(MSG_PKG&)));
	connect(&netcmd, SIGNAL(TagEpcReady(MSG_PKG&)), this, SLOT(OnMsgTagEpc(MSG_PKG&)));
}

iDataView::~iDataView()
{
}
void iDataView::OnRdmSelected(iRdm *rdm)
{
	//clear tags when rdm change selected
	if (tagModel->rowCount() > 0)
		tagModel->removeRows(0, tagModel->rowCount());

	//if (rdm)
	//{
	//	//test code
	//	Tags_Data tags;
	//	memset(&tags, 0, sizeof(tags));
	//	tags.Header.tagcount = 2;
	//	tags.Tags[0].uid = 12345;
	//	tags.Tags[0].temperature = 40;
	//	strcpy(tags.Tags[0].name, "ABCD");
	//	strcpy(tags.Tags[0].note, "Note1");
	//	tags.Tags[1].uid = 67890;
	//	tags.Tags[1].temperature = 50;
	//	strcpy(tags.Tags[1].name, "EFGH");
	//	strcpy(tags.Tags[1].note, "Note2");
	//	MSG_PKG msg;
	//	memcpy(msg.cmd_pkg.data, &tags, sizeof(tags));
	//	OnMsgTagsDataReady(msg);
	//}
}
void iDataView::OnMsgTagsDataReady(MSG_PKG& msg)
{
	//clear tags when new tags data cmd acked
	if (tagModel->rowCount() > 0)
		tagModel->removeRows(0, tagModel->rowCount());

	Tags_Data *tags = (Tags_Data *)msg.cmd_pkg.data;
	for (int i = 0; i < tags->Header.tagcount; i++)
	{
		if (tagModel->hasTag(tags->Tags[i].uid, tags->Tags[i].name))									//make sure no duplicated tags 
			continue;
		iTag *tag = new iTag(tags->Tags[i].uid, QString::fromLocal8Bit(tags->Tags[i].name));

		//todo: fill parameters of tag 
		tag->t_sid = tags->Tags[i].sid;
		tag->t_note = QString::fromLocal8Bit(tags->Tags[i].note);
		tag->t_temperature = (float)tags->Tags[i].temperature / 10.;
		tag->t_alarm = tags->Tags[i].alarm;
		tag->t_rssi = tags->Tags[i].rssi;
		tag->t_oc_rssi = tags->Tags[i].oc_rssi;
		tag->t_online = tags->Tags[i].online;

		tagModel->insertRow(0, tag);
	}
}
void iDataView::OnMsgTagEpc(MSG_PKG& msg)
{
	Tag_epc *tagEpc = (Tag_epc *)msg.cmd_pkg.data;

	tagModel->setTagEpc(tagEpc->uid, QString::fromLocal8Bit(tagEpc->epc));								//acked for epc change
}