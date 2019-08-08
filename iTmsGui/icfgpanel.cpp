#include "icfgpanel.h"
#include "irdm.h"
#include <QtGui>

iCfgPanel::iCfgPanel(QWidget *parent)
	: QTabWidget(parent), netcmd(EthernetCmd::Instance())
{
	ui.setupUi(this);

	setTabText(0,QString::fromLocal8Bit("ͨ������"));
	setTabText(1, QString::fromLocal8Bit("����������"));
	setTabText(2, QString::fromLocal8Bit("IoT����"));



	model = new QStandardItemModel(8, 4, this);
	//model->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("���"));
	model->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("ʶ���(UID)"));
	model->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("����(EPC)"));
	model->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("�����¶�"));
	model->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit("��ע"));

	ui.tableTags->setModel(model);
	//ui.tableTags->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableTags->setSelectionBehavior(QAbstractItemView::SelectRows);
	//ui.tableTags->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableTags->setAlternatingRowColors(true);

	QHeaderView *headerView = ui.tableTags->horizontalHeader();
	headerView->setStretchLastSection(true);

	setCurrentIndex(0);

	connect(&netcmd, SIGNAL(ModbusParamReady(MSG_PKG&)), this, SLOT(OnModbusParameters(MSG_PKG&)));
	connect(ui.btnRemoveTag, SIGNAL(clicked()), this, SLOT(OnRemoveTag()));

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
	int row = ui.tableTags->currentIndex().row();
	QModelIndex index = ui.tableTags->model()->index(row, 0, QModelIndex());
	ui.tableTags->edit(index);
}
void iCfgPanel::OnRdmSelected(iRdm *Rdm)
{
	ui.leRdmName->setText(Rdm->m_name);
	ui.leIPAddress->setText(Rdm->m_ip);
}