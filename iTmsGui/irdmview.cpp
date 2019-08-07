#include "irdmview.h"
#include <QtGui>
#include "Model.h"
#include <QSortFilterProxyModel>

iRdmView::iRdmView(QWidget *parent)
	: QWidget(parent)
	, m_Enetcmd(EthernetCmd::Instance())
{
	ui.setupUi(this);

	Rawrdmmodel = new RdmModel(this);

	rdmmodel = new QSortFilterProxyModel(this);
	rdmmodel->setSourceModel(Rawrdmmodel);

	ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableView->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableView->setAlternatingRowColors(true);

	QHeaderView *headerView = ui.tableView->horizontalHeader();
	headerView->setStretchLastSection(true);

	ui.tableView->setModel(rdmmodel);
	
	ui.btnDiscover->setText(QString::fromLocal8Bit("搜索设备"));
	ui.btnSave->setText(QString::fromLocal8Bit("保存参数"));
	ui.btnDownload->setText(QString::fromLocal8Bit("下载参数"));
	ui.btnUpgrade->setText(QString::fromLocal8Bit("升级设备"));

	connect(ui.tableView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(OnRdmTableActived(const QModelIndex &)));

	connect(ui.btnDiscover, SIGNAL(clicked()), this, SLOT(onbtndiscover()));
	connect(ui.btnDownload, SIGNAL(clicked()), this, SLOT(onbtnDownload()));

	connect(&m_Enetcmd, SIGNAL(newRdmready(MSG_PKG&)), this, SLOT(NewRdmfound(MSG_PKG&)));
}

iRdmView::~iRdmView()
{
}
void iRdmView::onbtndiscover()
{
	rdmmodel->removeRows(0, rdmmodel->rowCount());
	rdmsmap.clear();
	m_Enetcmd.discoverRdm();
}
void iRdmView::onbtnDownload()
{	
}
void iRdmView::NewRdmfound(MSG_PKG & msg)
{
	RDM_Paramters *rdm = (RDM_Paramters *)msg.cmd_pkg.data;
	QString name = rdm->RdmName;
	QString ip = rdm->RdmIp;
	QString mac = rdm->RdmMAC;
	
}
void iRdmView::OnRdmTableActived(const QModelIndex &index)
{
	
}