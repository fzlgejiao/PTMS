#include <QtWidgets/QApplication>
#include "iview.h"
#include "irdm.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	a.setApplicationVersion("V0.0.4");
	a.setApplicationName("PTMS-RDM");
	iView view;
	view.show();
	view.resize(800, 480);
	return a.exec();
}
