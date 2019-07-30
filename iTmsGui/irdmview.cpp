#include "irdmview.h"
#include <QtGui>

iRdmView::iRdmView(QWidget *parent)
	: QWidget(parent)
	, m_Enetcmd(EthernetCmd::Instance())
{
	ui.setupUi(this);

	model = new QStandardItemModel(8, 3, this);
	model->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("�豸����"));
	model->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("IP��ַ"));
	model->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("Mac��ַ"));

	ui.tableView->setModel(model);
	ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableView->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableView->setAlternatingRowColors(true);

	QHeaderView *headerView = ui.tableView->horizontalHeader();
	headerView->setStretchLastSection(true);


	ui.btnDiscover->setText(QString::fromLocal8Bit("�����豸"));
	ui.btnSave->setText(QString::fromLocal8Bit("�������"));
	ui.btnDownload->setText(QString::fromLocal8Bit("���ز���"));
	ui.btnUpgrade->setText(QString::fromLocal8Bit("�����豸"));

	connect(ui.btnDiscover, SIGNAL(clicked()), this, SLOT(onbtndiscover()));
	connect(ui.btnDownload, SIGNAL(clicked()), this, SLOT(onbtnDownload()));

	connect(&m_Enetcmd, SIGNAL(newRdmready(MSG_PKG&)), this, SLOT(NewRdmfound(MSG_PKG&)));
}

iRdmView::~iRdmView()
{
}
void iRdmView::onbtndiscover()
{
	model->clear();		
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

	model->setItem(0, 0, new QStandardItem(name));
	model->setItem(0, 1, new QStandardItem(ip));
	model->setItem(0, 2, new QStandardItem(mac));
}