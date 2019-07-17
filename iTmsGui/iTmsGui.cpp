#include <QtWidgets>
#include "iTmsGui.h"
#include "irdmview.h"
#include "icfgpanel.h"
#include "itagview.h"

iTmsGui::iTmsGui(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	//setup views
	QSplitter *splitter = new QSplitter;
	iRdmView *rdmview = new iRdmView;
	iCfgPanel *cfgpanel = new iCfgPanel;
	iTagView *tagview = new iTagView;

	QSplitter *splitterV = new QSplitter;
	splitterV->setOrientation(Qt::Vertical);
	splitterV->addWidget(cfgpanel);
	splitterV->addWidget(tagview);

	splitter->addWidget(rdmview);
	splitter->addWidget(splitterV);
	
	splitter->setStretchFactor(0, 0);
	splitter->setStretchFactor(1, 1);
	setCentralWidget(splitter);
}
