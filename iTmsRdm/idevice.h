#pragma once

#include <QObject>
#include <QtMqtt/QtMqtt>
#include <QMessageAuthenticationCode>
#include "itag.h"

enum OTADataFlag {
	OTA_Version		= 0x0001,
	OTA_Progress	= 0x0002,

};

class iRDM;
class iTag;
class iDevice : public QObject
{
	Q_OBJECT

public:
	iDevice(QObject *parent);
	~iDevice();

	void IOT_init();

	void PUB_tag_data(iTag* tag);
	void PUB_tag_event(iTag* tag);
	void PUB_rdm_data();
	void PUB_rdm_event();
	void PUB_ota_data(ushort flag);

protected:
	QString MakeJsonMessage(QString key, QJsonValue value);
	QJsonValue JsonParser(QString node, QString subnode, const QByteArray &message);
	void OTA_Process();

private:
	iRDM*			RDM;
	QMqttClient *	client;
	QString			PubParameterTopic;
	QString			PubParameterEvent;
	QString			SubParameterTopic;

	QString			PubOTAVersionTopic;
	QString			PubOTAProgressTopic;
	QString			SubOTARequestTopic;


private slots:
	void OnStateChanged(QMqttClient::ClientState);
	void OnMessageReceived(const QByteArray &message, const QMqttTopicName &topic = QMqttTopicName());
};
