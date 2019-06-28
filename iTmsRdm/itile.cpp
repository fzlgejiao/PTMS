#include "itile.h"
#include <QPainter> 
#include <QPaintEvent> 
#include "itag.h"

iTile::iTile(iTag* tag,QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	_tag = tag;

	QFont newFont = font();
	newFont.setPixelSize(30);
	//newFont.setBold(true);
	setFont(newFont);

	setWindowTitle("PTMS");
}

iTile::~iTile()
{
}
void iTile::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.fillRect(event->rect(), QBrush(Qt::white));

	//draw tag info
	//painter.setPen(Qt::darkGreen);
	//painter.setPen(Qt::DashLine);
	//painter.setBrush(Qt::NoBrush);
	//painter.drawRect(0, 0, 100, 100);
	if (!_tag)
		return;
	painter.save();
	//painter.setPen(Qt::blue);
	painter.drawText(10,50, QString("Tag%1 : %2").arg(_tag->T_sid).arg(_tag->T_temp, 0, 'f', 1));

	painter.restore();
}