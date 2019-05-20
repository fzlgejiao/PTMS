#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QFile>
#include "idevice.h"
#include "irdm.h"
typedef enum
{
	/* Burn firmware file failed */
	IOT_BURN_FAILED = -4,
    /* Check firmware file failed */
    IOT_CHECK_FALIED = -3,
    /* Fetch firmware file failed */
    IOT_FETCH_FAILED = -2,
    /* Initialized failed */
    IOT_GENERAL_FAILED = -1,

    /* [0, 100], percentage of fetch progress */
    IOT_OTAP_FETCH_PERCENTAGE_MIN = 0,
    IOT_OTAP_FETCH_PERCENTAGE_MAX = 100 
}OTA_PROGRESS;

class iDevice;
class iRDM;
class OTA : public QObject
{
	Q_OBJECT

public:
	OTA(QString &urlstr, QString & version, int size, QObject *parent=0);
	~OTA();
	void OTAstart();

private slots:
	void httpFinished();
	void httpReadyRead();
	void networkReplyProgress(qint64 bytesReceived, qint64 bytesTotal);
	void OnAuthenticationRequired(QNetworkReply *, QAuthenticator *authenticator);
	void sslErrors(QNetworkReply *, const QList<QSslError> &errors);

private:
	iDevice * iot;
	QString ota_url;
	QString ota_version;
	QString md5;
	int ota_size;
	
	QUrl url;
	QNetworkAccessManager qnam;
	QNetworkRequest request;
	QNetworkReply *reply;
	QString filename;
	QString directory;

	QFile *file;
	QFile *openFileForWrite(const QString &fileName);

	iRDM *rdm;
};
