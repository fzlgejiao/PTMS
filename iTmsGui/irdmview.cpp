#include "irdmview.h"
#include <QtGui>
#include "Model.h"
#include "irdm.h"
#include <QSortFilterProxyModel>

#define IP_COL	1

iRdmView::iRdmView(QWidget *parent)
	: QWidget(parent)
	, m_Enetcmd(EthernetCmd::Instance())
{
	ui.setupUi(this);

	//table rdms
	rdmmodel = new RdmModel(this);
		
	ui.tableRdms->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableRdms->setSelectionBehavior(QAbstractItemView::SelectRows);
	//ui.tableRdms->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableRdms->setAlternatingRowColors(true);

	QHeaderView *headerRdms = ui.tableRdms->horizontalHeader();
	headerRdms->setVisible(true);
	headerRdms->setStretchLastSection(true);
	ui.tableRdms->setModel(rdmmodel);
	
	//table online tags
	tagModel = new TagModel(this);
	//ui.tableTags->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableTags->setSelectionBehavior(QAbstractItemView::SelectRows);
	//ui.tableTags->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableTags->setAlternatingRowColors(true);

	QHeaderView *headerTags = ui.tableTags->horizontalHeader();
	headerTags->setVisible(true);
	headerTags->setStretchLastSection(true);

	ui.tableTags->setModel(tagModel);
	ui.tableTags->hideColumn(_Model::SID);
	ui.tableTags->hideColumn(_Model::TEMP);
	ui.tableTags->hideColumn(_Model::UPLIMIT);
	ui.tableTags->hideColumn(_Model::RSSI);
	ui.tableTags->hideColumn(_Model::OCRSSI);
	ui.tableTags->hideColumn(_Model::NOTE);

	ui.btnDiscover->setText(QString::fromLocal8Bit("搜索设备"));
	ui.btnSave->setText(QString::fromLocal8Bit("保存参数"));
	ui.btnDownload->setText(QString::fromLocal8Bit("下载参数"));
	ui.btnUpgrade->setText(QString::fromLocal8Bit("升级设备"));
	ui.btnSave->setEnabled(false);
	ui.btnDownload->setEnabled(false);
	ui.btnUpgrade->setEnabled(false);

	connect(ui.tableRdms->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(OnRdmSelectChanged(const QModelIndex &)));
	connect(ui.tableTags->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(OnTagSelectChanged(const QModelIndex &)));

	connect(ui.btnDiscover, SIGNAL(clicked()), this, SLOT(onbtndiscover()));
	connect(ui.btnDownload, SIGNAL(clicked()), this, SLOT(onbtnDownload()));

	connect(ui.btnFindTags, SIGNAL(clicked()), this, SLOT(OnbtnFindTags()));
	connect(ui.btnChangeEpc, SIGNAL(clicked()), this, SLOT(onbtnChangeEpc()));
	connect(ui.btnAddTag, SIGNAL(clicked()), this, SLOT(onbtnAddToSys()));

	connect(&m_Enetcmd, SIGNAL(newRdmReady(MSG_PKG&)), this, SLOT(NewRdmfound(MSG_PKG&)));
	connect(&m_Enetcmd, SIGNAL(OnlineTagsReady(MSG_PKG&)), this, SLOT(OnlineTagsFound(MSG_PKG&)));

	m_n2sTimerId = startTimer(2000);
}

iRdmView::~iRdmView()
{
}
void iRdmView::onbtndiscover()
{
	if (rdmmodel->rowCount() > 0)
		rdmmodel->removeRows(0, rdmmodel->rowCount());
	m_Enetcmd.UDP_discoverRdm();
}
void iRdmView::onbtnDownload()
{	

}
void iRdmView::OnbtnFindTags()
{
	if (tagModel->rowCount() > 0)
		tagModel->removeRows(0, tagModel->rowCount());

	//test code
	Tags_Online tags;
	tags.Header.tagcount = 2;
	tags.Tags[0].uid = 12345;
	strcpy(tags.Tags[0].name, "ABCD");
	tags.Tags[1].uid = 67890;
	strcpy(tags.Tags[1].name, "EFGH");
	MSG_PKG msg;
	memcpy(msg.cmd_pkg.data, &tags, sizeof(tags));
	OnlineTagsFound(msg);
}
void iRdmView::onbtnChangeEpc()
{
}
void iRdmView::onbtnAddToSys()
{
	iTag *tag = selectedTag();
	if (tag)
		emit tagAdded(tag);
}
void iRdmView::NewRdmfound(MSG_PKG & msg)
{
	RDM_Paramters *rdm = (RDM_Paramters *)msg.cmd_pkg.data;
	iRdm *newrdm = new iRdm(rdm->RdmName, rdm->RdmIp, rdm->RdmMAC, rdm->RdmVersion,this);

	rdmmodel->insertmyrow(0, newrdm);	
}
void iRdmView::OnlineTagsFound(MSG_PKG & msg)
{
	Tags_Online *tags = (Tags_Online *)msg.cmd_pkg.data;
	for (int i = 0; i < tags->Header.tagcount; i++)
	{
		iTag *tag = new iTag(tags->Tags[i].uid, tags->Tags[i].name);
		tagModel->insertRow(0, tag);
	}
}
void iRdmView::OnRdmSelectChanged(const QModelIndex & index)
{
	iRdm *current_rdm = selectedRdm();
	if (!current_rdm) 	
	{
		ui.btnDownload->setEnabled(false);
		ui.btnUpgrade->setEnabled(false);
		return;
	}
	ui.btnDownload->setEnabled(true);
	ui.btnUpgrade->setEnabled(true);

	m_Enetcmd.UDP_get_modbusparameters(current_rdm->m_ip);															//get modbus parameters
//	while(ui.tableTags->rowCount())
//		ui.tableTags->removeRow(0);

	emit RdmSelected(current_rdm);
}
void iRdmView::OnTagSelectChanged(const QModelIndex & index)
{
	iTag *tag = selectedTag();
	if (!tag)
	{
	}
}
iRdm* iRdmView::selectedRdm()
{
	QModelIndex index = ui.tableRdms->currentIndex();
	return (iRdm *)rdmmodel->data(index, Qt::UserRole).toUInt();
}
iTag* iRdmView::selectedTag()
{
	QModelIndex index = ui.tableTags->currentIndex();
	return (iTag *)tagModel->data(index, Qt::UserRole).toUInt();
}
void iRdmView::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == m_n2sTimerId)
	{

	}
	else
	{
		QObject::timerEvent(event);
	}
}