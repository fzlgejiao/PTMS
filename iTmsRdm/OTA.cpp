#include "OTA.h"
#include <QCoreApplication>
#include <QFileInfo>

#ifdef __linux__
#include <unistd.h>
#endif

OTA::OTA(QString &urlstr, QString & version, int size, QObject *parent)
	: QObject(parent)
{
	iot = (iDevice *)parent;

	rdm = (iRDM *)iot->parent();	

	ota_url = urlstr;
	ota_version = version;
	ota_size = size;

	directory = QCoreApplication::applicationDirPath()+"/";

	qDebug() << qnam.supportedSchemes()<<endl;
	qDebug() << QSslSocket::supportsSsl()<<endl;
	
	url = QUrl(ota_url);
	request.setUrl(url);

	connect(&qnam, &QNetworkAccessManager::authenticationRequired,this, &OTA::OnAuthenticationRequired);
	//connect(&qnam, &QNetworkAccessManager::sslErrors,this, &OTA::sslErrors);
}

OTA::~OTA()
{
}

void OTA::sslErrors(QNetworkReply *, const QList<QSslError> &errors)
{
	QString errorString;
	foreach(const QSslError &error, errors) {
		if (!errorString.isEmpty())
			errorString += '\n';
		errorString += error.errorString();
	}	
}

void OTA::OnAuthenticationRequired(QNetworkReply *, QAuthenticator *authenticator)
{
	//******************/
}


void OTA::OTAstart()
{		
	if (!url.isValid()) return;

	QString filename = url.fileName();
	if (filename.isEmpty())
	{
		rdm->Tmr_start();
		return;
	}

	file = openFileForWrite(directory+filename);
	if (!file)
	{
		rdm->Tmr_start();
		return;
	}
		
	reply = qnam.get(request);	
	reply->ignoreSslErrors();

	connect(reply, &QNetworkReply::finished, this, &OTA::httpFinished);
	connect(reply, &QIODevice::readyRead, this, &OTA::httpReadyRead);
	connect(reply, &QNetworkReply::downloadProgress, this, &OTA::networkReplyProgress);
}

void OTA::networkReplyProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	if (bytesTotal == 0) return;

	QString message = QString("Received %1bytes").arg(bytesReceived);

	int percent = bytesReceived * 100 / bytesTotal;
	//publish step and commment 
	if ((percent >= OTA_PROGRESS::IOT_OTAP_FETCH_PERCENTAGE_MIN) && (percent <= OTA_PROGRESS::IOT_OTAP_FETCH_PERCENTAGE_MAX))
	{
		qDebug() << "Received " << percent << "%" << endl;
		iot->PUB_ota_progress(percent, message);
	}
}

void OTA::httpFinished()
{	
	QFileInfo fi;
	if (file) {
		fi.setFile(file->fileName());
		file->close();
		delete file;
		file = NULL;
	}

	if (reply->error()) {
		QString message = QString("Error:%1").arg(reply->errorString());
		//publish error 
		int step = OTA_PROGRESS::IOT_FETCH_FAILED;
		iot->PUB_ota_progress(step, message);
		rdm->Tmr_start();
		return;
	}
	//download ok 
	//do : unzip the file for application file and restart 
	reply->deleteLater();
	reply = NULL;

	qDebug() << "Downloaded " << fi.size() << "bytes in file " << fi.fileName() << endl;
	if (fi.size() == ota_size)
	{			
#ifdef __linux__
	 //to do : in linux ,try to restart itself use the new app file
	 //ota file is the zip file ,unzip it in this directory check the file size and restart it
		//backup the old one
		QString appname = "iTmsRdm";
		QString backupcmd = QString("mv %1%2 %3%4.bk").arg(directory).arg(appname).arg(directory).arg(appname);
		system(backupcmd.toStdString().c_str());

		QString unzipcmd = QString("unzip -o %1 -d %2").arg(directory + fi.fileName()).arg(directory);
		system(unzipcmd.toStdString().c_str());

		QString xcmd = QString("chmod +x %1%2").arg(directory).arg(appname);
		system(xcmd.toStdString().c_str());

		qDebug() << "Restart the b2qt sevice" << endl;
		system("systemctl restart b2qt");
		//execl(directory+"/iTmsRdm", "iTmsRdm", NULL);
#endif
	}
}

void OTA::httpReadyRead()
{
	if (file)	
		file->write(reply->readAll());	
}

QFile *OTA::openFileForWrite(const QString &fileName)
{
	QScopedPointer<QFile> file(new QFile(fileName));
	if (!file->open(QIODevice::WriteOnly)) {
				return NULL;
	}
	return file.take();
}