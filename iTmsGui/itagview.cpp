#include "itagview.h"
#include "irdm.h"
#include <QtGui>

iTagView::iTagView(QWidget *parent)
	: QTableView(parent)
{
	ui.setupUi(this);

	model = new QStandardItemModel(8, 5, this);
	model->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("���"));
	model->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("����"));
	model->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("�¶�"));
	model->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit("����"));
	model->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit("����"));

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