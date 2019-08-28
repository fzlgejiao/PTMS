#include "itile.h"
#include <QPainter> 
#include <QPaintEvent> 
#include "itag.h"
#include "iview.h"

#define	TITLE_WIDTH	80

iTile::iTile(iTag* tag,QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	_tag = tag;

	QFont newFont = font();
	newFont.setPixelSize(20);
	newFont.setBold(true);
	setFont(newFont);

	connect(this, &iTile::tileDBClicked, (iView *)parent, &iView::OnTileDBClicked);
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

	int x = 20, y = 0;
	int w = this->rect().width() - 40;
	int h = this->rect().height()- 50;
	int w_t = w / 3;
	int w_v = w * 2 / 3;
	int h_t = 25;

	//draw title
	painter.save();
	//painter.setPen(Qt::blue);
	painter.drawText(QRect(QPoint(x,y),QSize(w,h_t)), Qt::AlignCenter, _tag->Title());
	y += 27;
	painter.drawLine(x, y, w+x, y);																	//H-line
	painter.drawLine(x+w_t-10, y, x+w_t-10, y+h);															//V-line
	painter.restore();


	//draw desc
	y += 5;
	QRect rtTitle = QRect(QPoint(x, y), QSize(w_t, h_t));
	QRect rtValue = QRect(QPoint(x + w_t, y), QSize(w_v, h_t));
	painter.save();
	painter.drawText(rtTitle, Qt::AlignLeft, QString::fromLocal8Bit("名称"));
	painter.drawText(rtValue, Qt::AlignLeft, QString("%1").arg(_tag->T_epc));
	painter.restore();

	//draw temperature
	painter.save();
	y += 35;
	rtTitle = QRect(QPoint(x, y), QSize(w_t, h_t));
	rtValue = QRect(QPoint(x + w_t, y), QSize(w_v, h_t));
	painter.drawText(rtTitle, Qt::AlignLeft, QString::fromLocal8Bit("温度"));
	if(_tag->isonline())
		painter.setPen(Qt::green);
	else
		painter.setPen(Qt::red);
	painter.drawText(rtValue, Qt::AlignLeft, QString("%1 %2C").arg(_tag->Temp(),-6).arg(QChar(0x00B0)));
	painter.restore();

	//draw RSSI
	painter.save();
	y += 35;
	rtTitle = QRect(QPoint(x, y), QSize(w_t, h_t));
	rtValue = QRect(QPoint(x + w_t, y), QSize(w_v, h_t));
	painter.drawText(rtTitle, Qt::AlignLeft, QString::fromLocal8Bit("信号"));
	if (_tag->isonline())
		painter.setPen(Qt::green);
	else
		painter.setPen(Qt::red);
	painter.drawText(rtValue, Qt::AlignLeft, QString("%1 dBm").arg(_tag->RSSI(),-6));
	painter.restore();

	painter.save();
	y += 35;
	rtTitle = QRect(QPoint(x, y), QSize(w_t, h_t));
	painter.drawText(rtTitle, Qt::AlignLeft, QString::fromLocal8Bit("告警"));
	if (_tag->isAlarm() == false)
	{
		painter.setPen(Qt::green);
		painter.setBrush(Qt::green);
	}
	else
	{
		painter.setPen(Qt::red);
		painter.setBrush(Qt::red);
	}
	rtValue = QRect(QPoint(x + w_t, y), QSize(w_v, h_t));
	painter.drawEllipse(rtValue.center(),10,10);
	painter.restore();
}
void iTile::OnDataChanged()
{
	update();
}
void iTile::mouseDoubleClickEvent(QMouseEvent * event)
{
	emit tileDBClicked(this);
}