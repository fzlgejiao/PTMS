#include "iTmsTest.h"
#include <QtWidgets/QApplication>
#include <QtSql/qsqldatabase.h>
#include <QtSql/qsqlerror.h>
#include <QMessageBox>
#include <QFile>
#include <QDebug> 
#include <QMutex> 
#include <QDateTime> 


void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	static QMutex mutex;
	mutex.lock();

	QString text;
	switch (type)
	{
	case QtDebugMsg:
		text = QString("Debug:");
		break;

	case QtWarningMsg:
		text = QString("Warning:");
		break;

	case QtCriticalMsg:
		text = QString("Critical:");
		break;

	case QtFatalMsg:
		text = QString("Fatal:");
	}

	QString context_info = QString("File:(%1) Line:(%2)").arg(QString(context.file)).arg(context.line);
	QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");
	QString current_date = QString("(%1)").arg(current_date_time);
	QString message = QString("%1 %2 %3 %4").arg(text).arg(context_info).arg(msg).arg(current_date);

	QFile file(qApp->applicationDirPath() + "/" + "log.txt");
	file.open(QIODevice::WriteOnly | QIODevice::Append);
	QTextStream text_stream(&file);
	text_stream << message << "\r\n";
	file.flush();
	file.close();

	mutex.unlock();
}

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
	a.setApplicationVersion("V0.2.0");

	qInstallMessageHandler(outputMessage);
	qDebug("*****************Application Start*****************");

	if (!createConnection())																		//open database
		return 1;

	iTmsTest w;
	w.show();
	w.resize(1200, 800);
	return a.exec();
}
