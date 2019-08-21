#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_iTmsGui.h"
#include "EthernetCmd.h"


class iRdmView;
class iCfgPanel;
class iTagView;
class QLabel;
class QProgressBar;
class iTmsGui : public QMainWindow
{
	Q_OBJECT

public:
	iTmsGui(QWidget *parent = Q_NULLPTR);

protected:
	void createStatusBar();
	virtual void closeEvent(QCloseEvent * event);

private:
	Ui::iTmsGuiClass ui;
	EthernetCmd &		EnetCmd;
	QLabel*				sBarVersion;
	QProgressBar*		m_progressbar;
	QLabel*				m_status;
	iRdmView *			rdmview;
	iCfgPanel *			cfgpanel;
	iTagView *			tagview;

private slots:
	void onupdateprogressbar(int value, int max);
	void onTransferStateChanged(FileTransferState state);
};
