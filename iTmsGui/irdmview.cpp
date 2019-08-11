#include "irdmview.h"
#include <QtGui>
#include "Model.h"
#include "irdm.h"
#include <QSortFilterProxyModel>


iRdmView::iRdmView(QWidget *parent)
	: QWidget(parent)
	, m_Enetcmd(EthernetCmd::Instance())
{
	ui.setupUi(this);

	//table rdms
	rdmmodel = new RdmModel(this);
		
	ui.tableRdms->setEditTriggers(QAbstractItemView::DoubleClicked);
	ui.tableRdms->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableRdms->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableRdms->setAlternatingRowColors(true);

	QHeaderView *headerRdms = ui.tableRdms->horizontalHeader();
	headerRdms->setVisible(true);
	headerRdms->setStretchLastSection(true);
	ui.tableRdms->setModel(rdmmodel);
	
	//table online tags
	tagModel = new TagModel(this);
	tagModel->setEditColumns(1 << _Model::EPC);
	
	ui.tableTags->setEditTriggers(QAbstractItemView::DoubleClicked);
	ui.tableTags->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableTags->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableTags->setAlternatingRowColors(true);

	QHeaderView *headerTags = ui.tableTags->horizontalHeader();
	headerTags->setVisible(true);
	headerTags->setStretchLastSection(true);

	ui.tableTags->setModel(tagModel);
	ui.tableTags->hideColumn(_Model::SID);
	ui.tableTags->hideColumn(_Model::TEMP);
	ui.tableTags->hideColumn(_Model::ALARM);
	ui.tableTags->hideColumn(_Model::UPLIMIT);
	ui.tableTags->hideColumn(_Model::RSSI);
	ui.tableTags->hideColumn(_Model::OCRSSI);
	ui.tableTags->hideColumn(_Model::NOTE);

	ui.btnChangeIP->setEnabled(false);
	ui.btnDownload->setEnabled(false);
	ui.btnUpgrade->setEnabled(false);
	ui.btnAddTag->setEnabled(false);
	ui.btnChangeEpc->setEnabled(false);

	connect(ui.tableRdms, SIGNAL(clicked(const QModelIndex &)), this, SLOT(OnRdmSelectChanged(const QModelIndex &)));//for test
	//connect(ui.tableRdms->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(OnRdmSelectChanged(const QModelIndex &)));
	connect(ui.tableTags->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(OnTagSelectChanged(const QModelIndex &)));
	connect(tagModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(OnTagDataChanged(const QModelIndex &)));

	connect(ui.btnDiscover, SIGNAL(clicked()), this, SLOT(onbtndiscover()));
	connect(ui.btnDownload, SIGNAL(clicked()), this, SLOT(onbtnDownload()));
	connect(ui.btnChangeIP, SIGNAL(clicked()), this, SLOT(onbtnChangeIP()));

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
	//clear rdms when discover rdms again
	if (rdmmodel->rowCount() > 0)
		rdmmodel->removeRows(0, rdmmodel->rowCount());

	//clear online tags when discover rdms again
	if (tagModel->rowCount() > 0)
		tagModel->removeRows(0, tagModel->rowCount());

	//to notify other views to update when rdm cleared before discover
	emit RdmSelected(NULL);

	m_Enetcmd.UDP_discoverRdm();
}
void iRdmView::onbtnDownload()
{	
	iRdm* rdm = selectedRdm();
	if (rdm)
		emit RdmDownloaded(rdm);
}
void iRdmView::onbtnChangeIP()
{
	int row = ui.tableRdms->currentIndex().row();
	QModelIndex index = ui.tableRdms->model()->index(row, _Model::IP, QModelIndex());
	ui.tableRdms->edit(index);
}
void iRdmView::OnbtnFindTags()
{
	if (tagModel->rowCount() > 0)
		tagModel->removeRows(0, tagModel->rowCount());

	iRdm *rdm = selectedRdm();
	if(rdm)
		m_Enetcmd.UDP_get_tagonline(rdm->m_ip);														//get online tags

	////test code
	//Tags_Online tags;
	//tags.Header.tagcount = 2;
	//tags.Tags[0].uid = 12345;
	//strcpy(tags.Tags[0].name, "ABCD");
	//tags.Tags[1].uid = 67890;
	//strcpy(tags.Tags[1].name, "EFGH");
	//MSG_PKG msg;
	//memcpy(msg.cmd_pkg.data, &tags, sizeof(tags));
	//OnlineTagsFound(msg);
}
void iRdmView::onbtnChangeEpc()
{
	int row = ui.tableTags->currentIndex().row();
	QModelIndex index = ui.tableTags->model()->index(row, _Model::EPC, QModelIndex());
	ui.tableTags->edit(index);
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
	iRdm *newrdm = new iRdm(rdm->RdmName, rdm->RdmIp, rdm->RdmMAC, rdm->RdmVersion,rdm->RdmNote,this);

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
	//clear online tags when rdm change selected
	if (tagModel->rowCount() > 0)
		tagModel->removeRows(0, tagModel->rowCount());


	iRdm *rdm = selectedRdm();
	if (!rdm)
	{
		ui.btnDownload->setEnabled(false);
		ui.btnUpgrade->setEnabled(false);
		ui.btnChangeIP->setEnabled(false);
	}
	else
	{
		ui.btnDownload->setEnabled(true);
		ui.btnUpgrade->setEnabled(true);
		ui.btnChangeIP->setEnabled(true);

		m_Enetcmd.UDP_get_modbusparameters(rdm->m_ip);												//get modbus parameters
		m_Enetcmd.UDP_get_tagonline(rdm->m_ip);														//get online tags

		////test code
		//Tags_Online tags;
		//tags.Header.tagcount = 2;
		//tags.Tags[0].uid = 12345;
		//strcpy(tags.Tags[0].name, "ABCD");
		//tags.Tags[1].uid = 67890;
		//strcpy(tags.Tags[1].name, "EFGH");
		//MSG_PKG msg;
		//memcpy(msg.cmd_pkg.data, &tags, sizeof(tags));
		//OnlineTagsFound(msg);
	}
		
	emit RdmSelected(rdm);

}
void iRdmView::OnTagSelectChanged(const QModelIndex & index)
{
	if (index.isValid())
	{
		ui.btnAddTag->setEnabled(true);
		ui.btnChangeEpc->setEnabled(true);
	}
	else
	{
		ui.btnAddTag->setEnabled(false);
		ui.btnChangeEpc->setEnabled(false);
	}

	iTag *tag = selectedTag();
	if (!tag)
	{
	}
}
void iRdmView::OnTagDataChanged(const QModelIndex &index)
{
	iTag *tag = (iTag *)index.data(Qt::UserRole).toUInt();
	if (tag)
	{
		//todo: send cmd to rdm to write epc, and get online tags again when write epc cmd acked
	}
}
iRdm* iRdmView::selectedRdm()
{
	QModelIndex index = ui.tableRdms->currentIndex();
	if (index.isValid() == false)
		return NULL;
	return (iRdm *)rdmmodel->data(index, Qt::UserRole).toUInt();
}
iTag* iRdmView::selectedTag()
{
	QModelIndex index = ui.tableTags->currentIndex();
	if (index.isValid() == false)
		return NULL;
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