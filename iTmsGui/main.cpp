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
	//QTextCodec::setCodecForCStrings(tc);															//���������Ҫ�������ַ���������QByteArray����QString����ʱʹ�õ�Ĭ�ϱ��뷽ʽ
	QTextCodec::setCodecForLocale(tc);																//���������Ҫ�������úͶԱ����ļ�ϵͳ��дʱ���Ĭ�ϱ����ʽ������ͨ������ȡһ���ļ�ʱ����ʱ�ı����ʽ������ͨ��qDebug()�����ӡ��Ϣʱ�ı���
	//QTextCodec::setCodecForTr(tc);																//������������������ô���tr����ʱ��Ĭ���ִ�����

	QApplication a(argc, argv);
	a.setApplicationVersion(QObject::tr("v0.0.4"));
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
