#pragma once

#include <QTableView>
#include "ui_itagview.h"

class QAbstractItemModel;
class iRdm;
class TagModel;
class iTagView : public QTableView
{
	Q_OBJECT

public:
	iTagView(QWidget *parent = Q_NULLPTR);
	~iTagView();

private:
	Ui::iTagView ui;
	TagModel *model;


public slots:
	void OnRdmSelected(iRdm *);
};
