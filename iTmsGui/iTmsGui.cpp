#include <QtWidgets>
#include "iTmsGui.h"
#include "irdmview.h"
#include "icfgpanel.h"
#include "idataview.h"
#include "irdm.h"
#include "Model.h"

#define MSG_TIMEOUT	2000

iTmsGui::iTmsGui(QWidget *parent)
	: QMainWindow(parent),EnetCmd(EthernetCmd::Instance())
{
	ui.setupUi(this);
	this->setWindowTitle(QString("PTMS - TOOL %1").arg(qApp->applicationVersion()));

	//setup views
	QSplitter *splitter = new QSplitter;
	rdmview		= new iRdmView;
	cfgpanel	= new iCfgPanel;
	dataView	= new iDataView;

	QSplitter *splitterV = new QSplitter;
	splitterV->setOrientation(Qt::Vertical);
	splitterV->addWidget(cfgpanel);
	splitterV->addWidget(dataView);

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

	connect(rdmview, SIGNAL(RdmSelected(iRdm *)),	dataView, SLOT(OnRdmSelected(iRdm *)));
	connect(cfgpanel,SIGNAL(RdmModified()),			rdmview, SLOT(OnRdmModified()));


	connect(&EnetCmd, SIGNAL(sendprogress(int, int)), this, SLOT(onupdateprogressbar(int, int)));
	connect(&EnetCmd, SIGNAL(transferstate(FileTransferState)), this, SLOT(onTransferStateChanged(FileTransferState)));

	
}
void iTmsGui::createStatusBar()
{
	sBarVersion = new QLabel(this);
	sBarVersion->setMinimumSize(120, 20);
	sBarVersion->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	sBarVersion->setText(QString::fromLocal8Bit("编译时间 : ") + tr(__DATE__) + "," + tr(__TIME__));
	sBarVersion->setFrameShape(QFrame::Panel);
	sBarVersion->setFrameShadow(QFrame::Sunken);


	//m_status= new QLabel(this);
	//m_status->setAlignment(Qt::AlignVCenter);
	//m_status->setText("Ready");

	m_progressbar = new QProgressBar(this);
	m_progressbar->setVisible(false);

	//statusBar()->insertWidget(0, m_status);
	statusBar()->insertPermanentWidget(1, m_progressbar);

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
		statusBar()->showMessage(QString::fromLocal8Bit("开始下载"), MSG_TIMEOUT);
	}
		break;

	case TransferError:
		statusBar()->showMessage(EnetCmd.errorstring(), MSG_TIMEOUT);
		break;

	case Finished:
	{
		statusBar()->showMessage(QString::fromLocal8Bit("下载成功"), MSG_TIMEOUT);
		m_progressbar->setVisible(false);
	}
	break;

	case Sending:
	{
		statusBar()->showMessage(QString::fromLocal8Bit("下载中..."), MSG_TIMEOUT);
	}
	break;
	
	default:
		statusBar()->showMessage("Ready", MSG_TIMEOUT);
	break;
	}

}
void iTmsGui::closeEvent(QCloseEvent * event)
{
	iRdm* rdm = rdmview->selectedRdm();
	if (rdm && rdm->isModified())
	{
		int ret = QMessageBox::warning(this, QString::fromLocal8Bit("PTMS"),
			QString::fromLocal8Bit("参数改变，需要下载参数到设备？"),
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		if (ret == QMessageBox::Yes)
		{
			event->ignore();
			//todo: download parameters to rdm
			cfgpanel->OnRdmDownloaded(rdm);
			return;
		}
		else if (ret == QMessageBox::Cancel)
		{
			event->ignore();
			return;
		}

	}
	event->accept();

}