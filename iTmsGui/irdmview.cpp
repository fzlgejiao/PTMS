#include "irdmview.h"
#include <QtGui>

iRdmView::iRdmView(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	model = new QStandardItemModel(8, 3, this);
	model->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("设备名称"));
	model->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("IP地址"));
	model->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("Mac地址"));

	ui.tableView->setModel(model);
	ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableView->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableView->setAlternatingRowColors(true);

	QHeaderView *headerView = ui.tableView->horizontalHeader();
	headerView->setStretchLastSection(true);


	ui.btnDiscover->setText(QString::fromLocal8Bit("搜索设备"));
	ui.btnSave->setText(QString::fromLocal8Bit("保存参数"));
	ui.btnDownload->setText(QString::fromLocal8Bit("下载参数"));
	ui.btnUpgrade->setText(QString::fromLocal8Bit("升级设备"));
}

iRdmView::~iRdmView()
{
}
