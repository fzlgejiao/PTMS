#include "itagview.h"
#include "irdm.h"
#include "Model.h"
#include <QtGui>

iTagView::iTagView(QWidget *parent)
	: QTableView(parent), netcmd(EthernetCmd::Instance())
{
	ui.setupUi(this);

	setStyleSheet("QTableView::item{selection-color: white; selection-background-color: rgb(20, 20, 125);}");

	model = new TagModel(this);

	this->setModel(model);
	this->setEditTriggers(QAbstractItemView::NoEditTriggers);
	this->setSelectionBehavior(QAbstractItemView::SelectRows);
	this->setSelectionMode(QAbstractItemView::SingleSelection);
	this->setAlternatingRowColors(true);

	QHeaderView *headerView = this->horizontalHeader();
	headerView->setStretchLastSection(true);

	this->hideColumn(_Model::UPLIMIT);
	this->setColumnWidth(_Model::SID, 70);
	this->setColumnWidth(_Model::UID, 180);

	connect(&netcmd, SIGNAL(TagsDataReady(MSG_PKG&)), this, SLOT(OnDataTagsReady(MSG_PKG&)));
	connect(&netcmd, SIGNAL(TagEpcReady(MSG_PKG&)), this, SLOT(OnTagEpc(MSG_PKG&)));
}

iTagView::~iTagView()
{
}
void iTagView::OnRdmSelected(iRdm *rdm)
{
	//clear tags when rdm change selected
	if (model->rowCount() > 0)
		model->removeRows(0, model->rowCount());

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
	//	OnDataTagsReady(msg);
	//}
}
void iTagView::OnDataTagsReady(MSG_PKG& msg)
{
	//clear tags when new tags data cmd acked
	if (model->rowCount() > 0)
		model->removeRows(0, model->rowCount());

	Tags_Data *tags = (Tags_Data *)msg.cmd_pkg.data;
	for (int i = 0; i < tags->Header.tagcount; i++)
	{
//		if (model->hasTag(tags->Tags[i].uid,tags->Tags[i].name))									//make sure no duplicated tags 
//			continue;
		iTag *tag = new iTag(tags->Tags[i].uid, QString::fromLocal8Bit(tags->Tags[i].name));

		//todo: fill parameters of tag 
		tag->t_sid = tags->Tags[i].sid;
		tag->t_note = QString::fromLocal8Bit(tags->Tags[i].note);
		tag->t_temperature = (float)tags->Tags[i].temperature/10.;
		tag->t_alarm = tags->Tags[i].alarm;
		tag->t_rssi = tags->Tags[i].rssi;
		tag->t_oc_rssi = tags->Tags[i].oc_rssi;
		tag->t_online = tags->Tags[i].online;

		model->insertRow(0, tag);
	}
}
void iTagView::OnTagEpc(MSG_PKG& msg)
{
	Tag_epc *tagEpc = (Tag_epc *)msg.cmd_pkg.data;

	model->setTagEpc(tagEpc->uid, QString::fromLocal8Bit(tagEpc->epc));								//acked for epc change
}