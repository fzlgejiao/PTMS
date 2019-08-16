#include <QtWidgets>
#include "iTmsGui.h"
#include "irdmview.h"
#include "icfgpanel.h"
#include "itagview.h"
#include "irdm.h"

iTmsGui::iTmsGui(QWidget *parent)
	: QMainWindow(parent),EnetCmd(EthernetCmd::Instance())
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

	createStatusBar();

	connect(rdmview, SIGNAL(RdmSelected(iRdm *)),	cfgpanel, SLOT(OnRdmSelected(iRdm *)));
	connect(rdmview, SIGNAL(RdmSaved(iRdm *)),		cfgpanel, SLOT(OnRdmSaved(iRdm *)));
	connect(rdmview, SIGNAL(RdmDownloaded(iRdm *)), cfgpanel, SLOT(OnRdmDownloaded(iRdm *)));
	connect(rdmview, SIGNAL(tagAdded(iTag *)),		cfgpanel, SLOT(OnTagAdded(iTag *)));

	connect(rdmview, SIGNAL(RdmSelected(iRdm *)),	tagview, SLOT(OnRdmSelected(iRdm *)));

	connect(&EnetCmd, SIGNAL(sendprogress(int, int)), this, SLOT(onupdateprogressbar(int, int)));
	connect(&EnetCmd, SIGNAL(transferstate(FileTransferState)), this, SLOT(onTransferStateChanged(FileTransferState)));

	
}
void iTmsGui::createStatusBar()
{
	sBarVersion = new QLabel(this);
	sBarVersion->setMinimumSize(120, 20);
	sBarVersion->setAlignment(Qt::AlignHCenter);
	sBarVersion->setText(qApp->applicationVersion());
	sBarVersion->setFrameShape(QFrame::Panel);
	sBarVersion->setFrameShadow(QFrame::Sunken);


	m_status= new QLabel(this);
	m_status->setAlignment(Qt::AlignVCenter);
	m_status->setText("Ready");

	m_progressbar = new QProgressBar(this);
	m_progressbar->setVisible(false);

	statusBar()->insertWidget(0, m_status);
	statusBar()->insertWidget(1, m_progressbar);

	statusBar()->insertPermanentWidget(0, sBarVersion);
}

void iTmsGui::onupdateprogressbar(int value, int max)
{
	m_progressbar->setMaximum(max);
	m_progressbar->setValue(value);
}
void iTmsGui::onTransferStateChanged(FileTransferState state)
{
	switch (state)
	{
	case Start:
	{
		m_progressbar->setVisible(true);
		m_status->setText(QString::fromLocal8Bit("开始下载"));
	}
		break;

	case TransferError:
		m_status->setText(EnetCmd.errorstring());
		break;

	case Finished:
	{
		m_status->setText(QString::fromLocal8Bit("下载成功"));
		m_progressbar->setVisible(false);
	}
	break;

	case Sending:
	{
		m_status->setText(QString::fromLocal8Bit("下载中..."));
	}
	break;
	
	default:
		m_status->setText("Ready");
	break;
	}

}