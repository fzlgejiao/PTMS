#include "irdmview.h"
#include <QtGui>

iRdmView::iRdmView(QWidget *parent)
	: QWidget(parent)
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
}

iRdmView::~iRdmView()
{
}
