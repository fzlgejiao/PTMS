#include "iTmsGui.h"
#include "irdm.h"
#include <QtWidgets/QApplication>
#include <QTextCodec>
#include <QStyle> 
#include <QDesktopWidget> 
#include <QtSql/qsqldatabase.h>
#include <QtSql/qsqlerror.h>
#include <QMessageBox>
#include <QFile>

bool createConnection()
{
	//file db
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
	QString dbfile = qApp->applicationDirPath() + "/" + "iTmsData.db";
	if (false == QFile::exists(dbfile))
		qDebug() << "File iTmsData.db does not exist.";
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
	QTextCodec *tc = QTextCodec::codecForName("GBK");
	//QTextCodec::setCodecForCStrings(tc);															//这个函数主要用在用字符常量或者QByteArray构造QString对象时使用的默认编码方式
	QTextCodec::setCodecForLocale(tc);																//这个函数主要用于设置和对本地文件系统读写时候的默认编码格式。比如通过流读取一个文件时内容时的编码格式。或者通过qDebug()输出打印信息时的编码
	//QTextCodec::setCodecForTr(tc);																//这个函数的作用是设置传给tr函数时的默认字串编码

	QApplication a(argc, argv);
	a.setApplicationVersion(QObject::tr("V0.2.0"));
	QString path = a.applicationDirPath();
	if (!createConnection())																		//open database
		return 1;

	iSys &oSys = iSys::Instance();

	iTmsGui w;
	w.show();
	w.resize(1200, 800);

	//Center Window on the Screen
	w.setGeometry(
		QStyle::alignedRect(
			Qt::LeftToRight,
			Qt::AlignCenter,
			w.size(),
			a.desktop()->availableGeometry()
		)
	);

	return a.exec();
}
