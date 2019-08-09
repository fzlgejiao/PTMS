#include "itagview.h"
#include "irdm.h"
#include "Model.h"
#include <QtGui>

iTagView::iTagView(QWidget *parent)
	: QTableView(parent)
{
	ui.setupUi(this);

	model = new TagModel(this);

	this->setModel(model);
	this->setEditTriggers(QAbstractItemView::NoEditTriggers);
	this->setSelectionBehavior(QAbstractItemView::SelectRows);
	this->setSelectionMode(QAbstractItemView::SingleSelection);
	this->setAlternatingRowColors(true);

	QHeaderView *headerView = this->horizontalHeader();
	headerView->setStretchLastSection(true);

	this->hideColumn(_Model::UPLIMIT);
}

iTagView::~iTagView()
{
}
void iTagView::OnRdmSelected(iRdm *Rdm)
{

}