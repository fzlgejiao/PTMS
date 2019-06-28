#include "iview.h"
#include "itile.h"
#include <QtWidgets/QGridLayout>
#include <QPainter> 
#include <QPaintEvent> 

iView::iView(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	QGridLayout *layout = new QGridLayout;

	int sid = 0;
	for (int i = 0; i < 2; ++i)
	{
		for (int j = 0; j < 3; ++j) {
			sid++;
			iTile *tile = new iTile(rdm.Tag_getbysid(sid),this);
			tiles[i * 3 + j] = tile;
			layout->addWidget(tile, i, j);
			//layout->addWidget(tiles[i], 1, i + 1);
		}
	}
	setLayout(layout);
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