#include "iTmsGui.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	iTmsGui w;
	w.show();
	return a.exec();
}
