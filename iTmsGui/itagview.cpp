#include "itagview.h"
#include "irdm.h"
#include <QtGui>

iTagView::iTagView(QWidget *parent)
	: QTableView(parent)
{
	ui.setupUi(this);

	model = new QStandardItemModel(8, 5, this);
	model->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("序号"));
	model->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("名称"));
	model->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("温度"));
	model->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit("报警"));
	model->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit("其他"));

	this->setModel(model);
	this->setEditTriggers(QAbstractItemView::NoEditTriggers);
	this->setSelectionBehavior(QAbstractItemView::SelectRows);
	this->setSelectionMode(QAbstractItemView::SingleSelection);
	this->setAlternatingRowColors(true);

	QHeaderView *headerView = this->horizontalHeader();
	headerView->setStretchLastSection(true);
}

iTagView::~iTagView()
{
}
void iTagView::OnRdmSelected(iRdm *Rdm)
{

}