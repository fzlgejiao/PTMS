#include "irdmview.h"
#include <QtGui>
#include "Model.h"
#include "irdm.h"
#include <QSortFilterProxyModel>
#include <QFileDialog> 
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QInputDialog> 

iRdmView::iRdmView(QWidget *parent)
	: QWidget(parent)
	, oSys(iSys::Instance())
	, netcmd(EthernetCmd::Instance())
{
	ui.setupUi(this);

	//table rdms
	rdmModel = new RdmModel(this);
	oSys.rdmModel = rdmModel;

	setStyleSheet("QTableView::item{selection-color: white; selection-background-color: rgb(20, 20, 125);}");

	ui.tableRdms->setEditTriggers(QAbstractItemView::DoubleClicked);
	ui.tableRdms->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableRdms->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableRdms->setAlternatingRowColors(true);

	QHeaderView *headerRdms = ui.tableRdms->horizontalHeader();
	headerRdms->setVisible(true);
	headerRdms->setStretchLastSection(true);
	ui.tableRdms->setModel(rdmModel);

	ui.tableRdms->setItemDelegateForColumn(_Model::IP, new IpAddressDelegate(this));


	//table online tags
	tagModel = new TagModel(this);
	tagModel->setEditColumns(1 << _Model::EPC);
	oSys.tagModelOnline = tagModel;

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
	ui.tableTags->setColumnWidth(_Model::UID, 180);

	ui.btnChangeIP->setEnabled(false);
	ui.btnDownload->setEnabled(false);
	ui.btnUpgrade->setEnabled(false);
	ui.btnAddTag->setEnabled(false);
	ui.btnChangeEpc->setEnabled(false);
	ui.btnFindTags->setEnabled(false);

	ui.tableTags->setItemDelegateForColumn(_Model::EPC, new LengthLimitDelegate(TAG_EPC_SIZE,false,this));

	//connect(ui.tableRdms, SIGNAL(clicked(const QModelIndex &)), this, SLOT(OnRdmSelectChanged(const QModelIndex &)));//for test
	connect(ui.tableRdms->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(OnRdmSelectChanged(const QModelIndex &)));
	connect(ui.tableTags->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(OnTagSelectChanged(const QModelIndex &)));
	connect(tagModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(OnTagDataChanged(const QModelIndex &)));
	connect(tagModel, SIGNAL(dataFailed(const QModelIndex &, const QString&)), this, SLOT(OnTagDataFailed(const QModelIndex &, const QString&)));

	connect(ui.btnDiscover, SIGNAL(clicked()), this, SLOT(onbtndiscover()));
	connect(ui.btnDownload, SIGNAL(clicked()), this, SLOT(onbtnDownload()));
	connect(ui.btnChangeIP, SIGNAL(clicked()), this, SLOT(onbtnChangeIP()));
	connect(ui.btnUpgrade, SIGNAL(clicked()), this, SLOT(onbtnUpgrade()));

	connect(ui.btnFindTags, SIGNAL(clicked()), this, SLOT(OnbtnFindTags()));
	connect(ui.btnChangeEpc, SIGNAL(clicked()), this, SLOT(onbtnChangeEpc()));
	connect(ui.btnAddTag, SIGNAL(clicked()), this, SLOT(onbtnAddToSys()));

	connect(&netcmd, SIGNAL(newRdmReady(MSG_PKG&)), this, SLOT(OnMsgRdmfound(MSG_PKG&)));
	connect(&netcmd, SIGNAL(TagsOnlineReady(MSG_PKG&)), this, SLOT(OnMsgOnlineTagsFound(MSG_PKG&)));
	connect(&netcmd, SIGNAL(TagEpcReady(MSG_PKG&)), this, SLOT(OnMsgTagEpc(MSG_PKG&)));

	connect(rdmModel, SIGNAL(IpChanged(iRdm *)), this, SLOT(OnRdmIpChanged(iRdm *)));
	
	m_n2sTimerId = startTimer(2000);
}

iRdmView::~iRdmView()
{
}
void iRdmView::onbtndiscover()
{
	//clear rdms when discover rdms again
	if (rdmModel->rowCount() > 0)
		rdmModel->removeRows(0, rdmModel->rowCount());

	//clear online tags when discover rdms again
	if (tagModel->rowCount() > 0)
		tagModel->removeRows(0, tagModel->rowCount());

	ui.btnChangeIP->setEnabled(false);
	ui.btnDownload->setEnabled(false);
	ui.btnUpgrade->setEnabled(false);
	ui.btnFindTags->setEnabled(false);

	//to notify other views to update when rdm cleared before discover
	emit RdmSelected(NULL);

	netcmd.UDP_discoverRdm();
}
void iRdmView::onbtnDownload()
{	
	iRdm* rdm = selectedRdm();
	if (rdm)
		emit RdmDownloaded(rdm);
	ui.btnDownload->setEnabled(false);

}
void iRdmView::onbtnChangeIP()
{
	int row = ui.tableRdms->currentIndex().row();
	QModelIndex index = ui.tableRdms->model()->index(row, _Model::IP, QModelIndex());
	ui.tableRdms->edit(index);
}
void iRdmView::OnbtnFindTags()
{
	iRdm *rdm = selectedRdm();
	if(rdm)
		netcmd.UDP_get_tagsonline(rdm);																//get online tags

	////test code
	//Tags_Online tags;
	//tags.Header.tagcount = 2;
	//tags.Tags[0].uid = 12345;
	//strcpy(tags.Tags[0].name, "ABCD");
	//tags.Tags[1].uid = 67890;
	//strcpy(tags.Tags[1].name, "EFGH");
	//MSG_PKG msg;
	//memcpy(msg.cmd_pkg.data, &tags, sizeof(tags));
	//OnMsgOnlineTagsFound(msg);
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
void iRdmView::onbtnUpgrade()
{
	iRdm* rdm = selectedRdm();
	if (!rdm) return;

	QString tarfilename = QFileDialog::getOpenFileName(this, tr("Choose Upgrade file"), ".", tr("Gzip File (*.gz)"));
	if (tarfilename.isEmpty()) return;

	netcmd.UDP_fileinfo(rdm, tarfilename, TarFile);
}


void iRdmView::OnMsgRdmfound(MSG_PKG & msg)
{
	RDM_Paramters *rdm = (RDM_Paramters *)msg.cmd_pkg.data;
	//to check if the ram is existing or not
	if (rdmModel->hasRdm(rdm->RdmMAC))
		return;
	iRdm *newrdm = new iRdm(QString::fromLocal8Bit(rdm->RdmName), rdm->RdmIp, rdm->RdmMAC, rdm->RdmVersion, QString::fromLocal8Bit(rdm->RdmNote),this);
	newrdm->m_comname = rdm->RdmComName;
	rdmModel->insertmyrow(0, newrdm);	
	//todo: select the first rdm
	if (rdmModel->rowCount() == 1)
	{
		//QItemSelectionModel *selectionModel = ui.tableRdms->selectionModel();
		QModelIndex index = ui.tableRdms->model()->index(0, 0, QModelIndex());
		//selectionModel->select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
		ui.tableRdms->setCurrentIndex(index);
	}
}
void iRdmView::OnMsgOnlineTagsFound(MSG_PKG & msg)
{
	//clear tags list before updating tag list
	if (tagModel->rowCount() > 0)
		tagModel->removeRows(0, tagModel->rowCount());

	//update tag list
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
		ui.btnFindTags->setEnabled(false);
	}
	else
	{
		ui.btnDownload->setEnabled(false);
		ui.btnUpgrade->setEnabled(true);
		ui.btnChangeIP->setEnabled(true);
		ui.btnFindTags->setEnabled(true);

		netcmd.UDP_get_iotparameters(rdm);														//get iot parameters
		netcmd.UDP_get_modbusparameters(rdm);													//get modbus parameters
		netcmd.UDP_get_tagsonline(rdm);															//get online tags
		netcmd.UDP_get_tagspara(rdm);															//get managed tags para
		netcmd.UDP_get_tagsdata(rdm);															//get managed tags data

		////test code
		//Tags_Online tags;
		//tags.Header.tagcount = 2;
		//tags.Tags[0].uid = 12345;
		//strcpy(tags.Tags[0].name, "ABCD");
		//tags.Tags[1].uid = 67890;
		//strcpy(tags.Tags[1].name, "EFGH");
		//MSG_PKG msg;
		//memcpy(msg.cmd_pkg.data, &tags, sizeof(tags));
		//OnMsgOnlineTagsFound(msg);
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
		if (index.column() == _Model::EPC)
		{
			iRdm *rdm = selectedRdm();
			if(rdm)
				netcmd.UDP_set_tagepc(rdm, tag);
		}
	}
}
void iRdmView::OnTagDataFailed(const QModelIndex &index, const QString& error)
{
	if (index.column() == _Model::EPC)
	{
		QMessageBox mbx(QMessageBox::Warning, "PTMS", error, QMessageBox::Ok);
		mbx.setMinimumSize(600, 400);
		mbx.exec();
	}
}

iRdm* iRdmView::selectedRdm()
{
	QModelIndex index = ui.tableRdms->currentIndex();
	if (index.isValid() == false)
		return NULL;
	return (iRdm *)rdmModel->data(index, Qt::UserRole).toUInt();
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
		iRdm *rdm = selectedRdm();
		if (rdm)
			netcmd.UDP_get_tagsdata(rdm);															//get managed tags data
	}
	else
	{
		QObject::timerEvent(event);
	}
}

void iRdmView::OnRdmModified()
{
	iRdm* rdm = selectedRdm();
	if (rdm)
	{
		rdm->setModified(true);
		ui.btnDownload->setEnabled(true);
	}
}
void iRdmView::OnRdmIpChanged(iRdm *rdm)
{
	netcmd.UDP_ipset(rdm);
}
void iRdmView::OnMsgTagEpc(MSG_PKG& msg)
{
	Tag_epc *tagEpc = (Tag_epc *)msg.cmd_pkg.data;

	tagModel->setTagEpc(tagEpc->uid, QString::fromLocal8Bit(tagEpc->epc));							//acked for epc change
}