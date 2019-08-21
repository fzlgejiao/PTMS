#include "icfgdlg.h"
#include "irdm.h"
#include "ibc.h"

iCfgDlg::iCfgDlg(iRDM* rdm, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	RDM = rdm;

	QString style = "QListWidget::item { min-height: 40px; min-width: 60px; }";

	ui.listWidget->setStyleSheet(style);

	ui.leRdmName->setText(RDM->RDM_name);
	ui.leVersion->setText(qApp->applicationVersion());
	ui.leRdmIP->setText(RDM->bc->getIP());
	ui.leRdmMac->setText(RDM->bc->getMAC());

	//modbus
	ui.rbtnRTU->setChecked(RDM->modbustype == "RTU" ? true : false);
	ui.leModbusAddr->setText(QString::number(RDM->rtuslaveaddress));
	ui.leModbusPort->setText(QString::number(RDM->TcpPort));

	for(int i = 1;i <= RDM->Tag_count();i++)
		ui.listWidget->addItem(RDM->Tag_getbysid(i)->Title());
	connect(ui.listWidget,SIGNAL(currentRowChanged(int)),this, SLOT(changePage(int)));
	connect(ui.btnClose, SIGNAL(clicked(bool)), this, SLOT(close()));

	ui.listWidget->setCurrentRow(0);
}

iCfgDlg::~iCfgDlg()
{
}
void iCfgDlg::changePage(int row)
{

	iTag *tag = RDM->Tag_getbysid(row+1);
	if (tag)
	{
		ui.leTagDesc->setText(tag->T_epc);
		ui.leTagTempLimit->setText(QString::number(tag->T_uplimit));
		ui.leTagTemp->setText(tag->Temp());
		ui.lbTempUnit->setText(QString("%1C").arg(QChar(0x00B0)));
		ui.leTagRSSI->setText(tag->RSSI());
		ui.leTagNote->setText(tag->T_note);
	}

}