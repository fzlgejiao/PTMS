#include "irdmview.h"
#include <QtGui>
#include "Model.h"
#include <QSortFilterProxyModel>

#define IP_COL	1

iRdmView::iRdmView(QWidget *parent)
	: QWidget(parent)
	, m_Enetcmd(EthernetCmd::Instance())
{
	ui.setupUi(this);

	rdmmodel = new RdmModel(this);
		

	ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableView->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableView->setAlternatingRowColors(true);

	QHeaderView *headerView = ui.tableView->horizontalHeader();
	headerView->setStretchLastSection(true);

	ui.tableView->setModel(rdmmodel);
	
	ui.btnDiscover->setText(QString::fromLocal8Bit("�����豸"));
	ui.btnSave->setText(QString::fromLocal8Bit("�������"));
	ui.btnDownload->setText(QString::fromLocal8Bit("���ز���"));
	ui.btnUpgrade->setText(QString::fromLocal8Bit("�����豸"));
	ui.btnSave->setEnabled(false);
	ui.btnDownload->setEnabled(false);
	ui.btnUpgrade->setEnabled(false);

	connect(ui.tableView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(OnRdmSelectChanged(const QModelIndex &)));

	connect(ui.btnDiscover, SIGNAL(clicked()), this, SLOT(onbtndiscover()));
	connect(ui.btnDownload, SIGNAL(clicked()), this, SLOT(onbtnDownload()));

	connect(&m_Enetcmd, SIGNAL(newRdmReady(MSG_PKG&)), this, SLOT(NewRdmfound(MSG_PKG&)));
//	connect(ui.tableWidget, SIGNAL(currentItemChanged(QTableWidgetItem *, QTableWidgetItem *)), this, SLOT(OnRdmSelectChanged()));

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
void iRdmView::NewRdmfound(MSG_PKG & msg)
{
	RDM_Paramters *rdm = (RDM_Paramters *)msg.cmd_pkg.data;
	QString name = rdm->RdmName;
	QString ip = rdm->RdmIp;
	QString mac = rdm->RdmMAC;
	CRdm *newrdm = new CRdm(name, ip, mac, this);

	rdmmodel->insertmyrow(0, newrdm);	
}
void iRdmView::OnRdmSelectChanged(const QModelIndex & index)
{
	CRdm *current_rdm = (CRdm *)rdmmodel->data(index, Qt::UserRole).toUInt();
	if (!current_rdm) 	
	{
		ui.btnDownload->setEnabled(false);
		ui.btnUpgrade->setEnabled(false);
		return;
	}
	ui.btnDownload->setEnabled(true);
	ui.btnUpgrade->setEnabled(true);

	m_Enetcmd.UDP_get_modbusparameters(current_rdm->m_ip);															//get modbus parameters
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