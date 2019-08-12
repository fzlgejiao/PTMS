#include "iview.h"
#include "irdm.h"
#include "ireader.h"
#include "itile.h"
#include "itag.h"
#include "icfgdlg.h"
#include <QtWidgets/QGridLayout>
#include <QPainter> 
#include <QPaintEvent> 
#include <QStyle> 
#include <QDesktopWidget> 

iView::iView(iRDM* rdm,QWidget *parent)
	: QWidget(parent), RDM(iRDM::Instance())
{
	ui.setupUi(this);

	QGridLayout *layout = new QGridLayout;
	int cols = 3;
	int rows = ceil(RDM.taglist.count() / (float)cols);
	for (iTag *tag : RDM.taglist)
	{
		iTile *tile = new iTile(tag, this);
		if(tile)
			tilelist.insert(tag->T_sid, tile);
	}
	for (int i = RDM.taglist.count() + 1; i <= rows*cols; i++)
	{
		iTile *tile = new iTile(NULL, this);
		if (tile)
			tilelist.insert(i, tile);
	}
	int sid = 0;
	for (int i = 0; i < rows; ++i)
	{
		for (int j = 0; j < cols; ++j) {
			sid++;
			iTile *tile = Tile(sid);
			if(tile)
				layout->addWidget(tile, i, j);
		}
	}
	setLayout(layout);

	setWindowTitle(qApp->applicationName());

	connect(RDM.reader, &iReader::tagUpdated, this, &iView::OnTagUpdated);
}

iView::~iView()
{
}
void iView::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.fillRect(event->rect(), QBrush(Qt::magenta));
}
void iView::OnTagUpdated(iTag* tag)
{
	iTile *tile = Tile(tag->T_sid);
	if (tile)
		tile->update();
}
void iView::OnTileDBClicked(iTile *)
{
	iCfgDlg dlg(this);
	//Center Window on the Screen
	dlg.setGeometry(
		QStyle::alignedRect(
			Qt::LeftToRight,
			Qt::AlignCenter,
			dlg.size(),
			qApp->desktop()->availableGeometry()
		)
	);

	dlg.exec();
}