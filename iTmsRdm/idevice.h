#pragma once

#include <QObject>
#include <QtMqtt/QtMqtt>
#include <QMessageAuthenticationCode>

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

protected:
	QString MakeJsonMessage(QString key, QJsonValue value);
	QJsonValue JsonParser(QString node, QString subnode, const QByteArray &message);
	void OTA_Process();

private:
	iRDM*			RDM;
	QMqttClient *	client;
	QString			PUB_topic;
	QString			PUB_event;
	QString			PubOTAVersionTopic;
	QString			PubOTAProgressTopic;
	QString			SubParameterTopic;
	QString			SubOTARequestTopic;


private slots:
	void OnStateChanged();
	void OnMessageReceived(const QByteArray &message, const QMqttTopicName &topic = QMqttTopicName());
};
