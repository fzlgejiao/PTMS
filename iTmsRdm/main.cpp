#include <QtCore/QCoreApplication>
#include "irdm.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	iRDM rdm;

	return a.exec();
}
