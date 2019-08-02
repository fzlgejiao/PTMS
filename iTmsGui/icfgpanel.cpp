#include "icfgpanel.h"
#include <QtGui>

iCfgPanel::iCfgPanel(QWidget *parent)
	: QTabWidget(parent)
{
	ui.setupUi(this);

	setTabText(0,QString::fromLocal8Bit("通用设置"));
	setTabText(1, QString::fromLocal8Bit("传感器设置"));
	setTabText(2, QString::fromLocal8Bit("IoT设置"));



	model = new QStandardItemModel(8, 5, this);
	model->setHeaderData(0, Qt::Horizontal, QString::fromLocal8Bit("序号"));
	model->setHeaderData(1, Qt::Horizontal, QString::fromLocal8Bit("识别号"));
	model->setHeaderData(2, Qt::Horizontal, QString::fromLocal8Bit("名称"));
	model->setHeaderData(3, Qt::Horizontal, QString::fromLocal8Bit("报警温度"));
	model->setHeaderData(4, Qt::Horizontal, QString::fromLocal8Bit("备注"));

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
