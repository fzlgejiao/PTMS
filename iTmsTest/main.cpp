#include "iTmsTest.h"
#include <QtWidgets/QApplication>
#include <QtSql/qsqldatabase.h>
#include <QtSql/qsqlerror.h>
#include <QMessageBox>
#include <QFile>
#include <QDebug> 

bool createConnection()
{
	//file db
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
	QString dbfile = qApp->applicationDirPath() + "/" + "iTmsTest.db";
	if (false == QFile::exists(dbfile))
		qDebug() << "File iTmsTest.db does not exist.";
	db.setDatabaseName(dbfile);
	if (!db.open()) {
		QMessageBox::critical(0, qApp->applicationName(),
			db.lastError().text());
		return false;
	}
	return true;
}
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	a.setApplicationName(QObject::tr("PTMS - Test"));
	a.setApplicationVersion("0.0.2");

	if (!createConnection())																		//open database
		return 1;

	iTmsTest w;
	w.show();
	w.resize(1200, 800);
	return a.exec();
}
