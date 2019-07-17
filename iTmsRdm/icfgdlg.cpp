#include "icfgdlg.h"
#include "irdm.h"

iCfgDlg::iCfgDlg(QWidget *parent)
	: QDialog(parent), RDM(iRDM::Instance())
{
	ui.setupUi(this);

	QString style = "QListWidget::item { min-height: 40px; min-width: 60px; }";

	ui.listWidget->setStyleSheet(style);

	ui.listWidget->addItem("General");
	for(int i = 1;i <= RDM.Tag_count();i++)
		ui.listWidget->addItem(RDM.Tag_getbysid(i)->Title());
	connect(ui.listWidget,SIGNAL(currentRowChanged(int)),this, SLOT(changePage(int)));
	connect(ui.btnClose, SIGNAL(clicked(bool)), this, SLOT(close()));

	ui.listWidget->setCurrentRow(0);
}

iCfgDlg::~iCfgDlg()
{
}
void iCfgDlg::changePage(int row)
{
	if (row == 0)
	{
		ui.stackedWidget->setCurrentIndex(0);
		ui.lbTitle->setText("General");
		ui.leVersion->setText(qApp->applicationVersion());

	}
	else
	{
		ui.stackedWidget->setCurrentIndex(1);
		iTag *tag = RDM.Tag_getbysid(row);
		if (tag)
		{
			ui.lbTitle->setText(tag->Title());
			ui.leTagDesc->setText(tag->T_epc);
			ui.leTagTemp->setText(tag->Temp());
			ui.lbTempUnit->setText(QString("%1C").arg(QChar(0x00B0)));
			ui.leTagRSSI->setText(tag->RSSI());
		}
	}

}