#include <QtCore/QCoreApplication>
#include "irdm.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	a.setApplicationVersion("V0.0.4");
	a.setApplicationName("PTMS-RDM");
	iRDM rdm;

	return a.exec();
}
