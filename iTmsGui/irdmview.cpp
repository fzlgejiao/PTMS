#include "irdmview.h"
#include <QtGui>

#define IP_COL	1

iRdmView::iRdmView(QWidget *parent)
	: QWidget(parent)
	, m_Enetcmd(EthernetCmd::Instance())
{
	ui.setupUi(this);

	ui.tableWidget->setRowCount(8);
	//ui.tableWidget->setColumnCount(3);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableWidget->setAlternatingRowColors(true);

	QHeaderView *headerView = ui.tableWidget->horizontalHeader();
	headerView->setStretchLastSection(true);


	ui.btnDiscover->setText(QString::fromLocal8Bit("搜索设备"));
	ui.btnSave->setText(QString::fromLocal8Bit("保存参数"));
	ui.btnDownload->setText(QString::fromLocal8Bit("下载参数"));
	ui.btnUpgrade->setText(QString::fromLocal8Bit("升级设备"));
	ui.btnSave->setEnabled(false);
	ui.btnDownload->setEnabled(false);
	ui.btnUpgrade->setEnabled(false);

	connect(ui.btnDiscover, SIGNAL(clicked()), this, SLOT(onbtndiscover()));
	connect(ui.btnDownload, SIGNAL(clicked()), this, SLOT(onbtnDownload()));

	connect(&m_Enetcmd, SIGNAL(newRdmReady(MSG_PKG&)), this, SLOT(NewRdmfound(MSG_PKG&)));
	connect(ui.tableWidget, SIGNAL(currentItemChanged(QTableWidgetItem *, QTableWidgetItem *)), this, SLOT(OnRdmSelectChanged()));

	m_n2sTimerId = startTimer(2000);
}

iRdmView::~iRdmView()
{
}
void iRdmView::onbtndiscover()
{
	ui.tableWidget->clearContents();
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

	QTableWidgetItem *item;
	item = new QTableWidgetItem;
	ui.tableWidget->setItem(0, 0,item);
	item->setText(name);
	item = new QTableWidgetItem;
	ui.tableWidget->setItem(0, 1, item);
	item->setText(ip);
	item = new QTableWidgetItem;
	ui.tableWidget->setItem(0, 2, item);
	item->setText(mac);
}
void iRdmView::OnRdmSelectChanged()
{
	QString IP = currentRdm();
	if (IP.isEmpty())
	{
		ui.btnDownload->setEnabled(false);
		ui.btnUpgrade->setEnabled(false);
		return;
	}
	ui.btnDownload->setEnabled(true);
	ui.btnUpgrade->setEnabled(true);

	m_Enetcmd.UDP_get_modbusparameters(IP);															//get modbus parameters
}
QString iRdmView::currentRdm()
{
	int row = ui.tableWidget->currentRow();
	QTableWidgetItem *item = ui.tableWidget->item(row, IP_COL);										//to get ip address item data
	if (!item)
		return QString();
	return item->text();
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