#pragma once

#include <QTableView>
#include "ui_itagview.h"
#include "EthernetCmd.h"

class QAbstractItemModel;
class iRdm;
class TagModel;
class iTag;
class iTagView : public QTableView
{
	Q_OBJECT

public:
	iTagView(QWidget *parent = Q_NULLPTR);
	~iTagView();

private:
	Ui::iTagView ui;
	TagModel *model;
	EthernetCmd &netcmd;

public slots:
	void OnRdmSelected(iRdm *);
	void OnDataTagsReady(MSG_PKG&);
};
