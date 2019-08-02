#include "icfgpanel.h"
#include <QtGui>

iCfgPanel::iCfgPanel(QWidget *parent)
	: QTabWidget(parent)
{
	ui.setupUi(this);

	setTabText(0,QString::fromLocal8Bit("ͨ������"));
	setTabText(1, QString::fromLocal8Bit("����������"));
	setTabText(2, QString::fromLocal8Bit("IoT����"));



	model = new QStandardItemModel(8, 5, this);
	model->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("���"));
	model->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("ʶ���"));
	model->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("����"));
	model->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit("�����¶�"));
	model->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit("��ע"));

	ui.tableTags->setModel(model);
	//ui.tableTags->setEditTriggers(QAbstractItemView::NoEditTriggers);
	//ui.tableTags->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableTags->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableTags->setAlternatingRowColors(true);

	QHeaderView *headerView = ui.tableTags->horizontalHeader();
	headerView->setStretchLastSection(true);
}

iCfgPanel::~iCfgPanel()
{
}
