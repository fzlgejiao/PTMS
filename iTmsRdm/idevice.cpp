#include "idevice.h"
#include "irdm.h"
#include "itag.h"

iDevice::iDevice(QObject *parent)
	: QObject(parent)
{
	RDM = (iRDM *)parent;

	client = new QMqttClient();

	connect(client, &QMqttClient::stateChanged, this, &iDevice::OnStateChanged);
	connect(client, &QMqttClient::messageReceived, this, &iDevice::OnMessageReceived);
}

iDevice::~iDevice()
{
}
void iDevice::IOT_init()
{
	//init MQTT client
	client->disconnect();

	QMap<QCryptographicHash::Algorithm, QString> signmethodmap;
	signmethodmap.insert(QCryptographicHash::Md5, "hmacmd5");
	signmethodmap.insert(QCryptographicHash::Sha1, "hmacsha1");
	signmethodmap.insert(QCryptographicHash::Sha256, "hmacsha256");

	QCryptographicHash::Algorithm signmethod = QCryptographicHash::Sha1;

	QString hostServer = QString("%1.iot-as-mqtt.%2.aliyuncs.com").arg(RDM->productkey).arg(RDM->regionid);
	QString clientId = "123";						//you can use ethernet MAC address	or your SN
	QString timestamp = QString::number(QDateTime::currentDateTime().toTime_t());

	QString signcontent = QString("clientId%1deviceName%2productKey%3timestamp%4").arg(clientId).arg(RDM->devicename).arg(RDM->productkey).arg(timestamp);
	QString	mqttPassword = QMessageAuthenticationCode::hash(signcontent.toUtf8(), RDM->devicesecret.toUtf8(), signmethod).toHex();	//use DeviceSecre to Key	
	QString mqttUserName = RDM->devicename + "&" + RDM->productkey;
	QString mqttClientId = clientId + "|securemode=3,signmethod=" + signmethodmap.value(signmethod) + ",timestamp=" + timestamp + "|";


	client->setHostname(hostServer);
	client->setPort(1883);
	client->setClientId(mqttClientId);
	client->setUsername(mqttUserName);
	client->setPassword(mqttPassword);

	client->connectToHost();

	PubParameterTopic = QString("/sys/%1/%2/thing/event/property/post").arg(RDM->productkey).arg(RDM->devicename);
	PubParameterEvent = QString("/sys/%1/%2/thing/event/{tsl.event.TemperatureAlarm}/post").arg(RDM->productkey).arg(RDM->devicename);

	PubOTAVersionTopic = QString("/ota/device/inform/%1/%2").arg(RDM->productkey).arg(RDM->devicename);
	PubOTAProgressTopic = QString("/ota/device/progress/%1/%2").arg(RDM->productkey).arg(RDM->devicename);

	SubParameterTopic = QString("/sys/%1/%2/thing/service/property/set").arg(RDM->productkey).arg(RDM->devicename);
	SubOTARequestTopic = QString("/ota/device/upgrade/%1/%2").arg(RDM->productkey).arg(RDM->devicename);
}
void iDevice::OnStateChanged()
{
	QMqttClient::ClientState status = client->state();
	if (status == QMqttClient::Connected)					//if first connected, publish my version
	{
		QString version = "V1.0.0";
		QString MESSAGE_FORMAT = QString("{\"id\":3,\"params\":{\"version\":\"%1\"},\"method\":\"thing.event.property.post\"}").arg(version);
		client->publish(PubOTAVersionTopic, MESSAGE_FORMAT.toUtf8());
		//if connected, subscription the topics
		client->subscribe(SubParameterTopic);
		client->subscribe(SubOTARequestTopic);
	}
}
void iDevice::OnMessageReceived(const QByteArray &message, const QMqttTopicName &topic)
{
	QString messagestr = QString(message);
	QString topicname = topic.name();
	if (topicname == SubParameterTopic)
	{
		//Parameter settings		
		QJsonValue idjson = JsonParser("id", NULL, message);
		if (idjson.isDouble())
		{
			int id = idjson.toVariant().toInt();
		}
		QJsonValue limitjson = JsonParser("params", "Tag1_Upperlimit", message);
		if (limitjson.isDouble())
		{
			float tag1_limit = limitjson.toDouble();
		}
	}
	else if (topicname == SubOTARequestTopic)
	{
		OTA_Process();
	}
}
QJsonValue iDevice::JsonParser(QString node, QString subnode, const QByteArray &message)
{
	QJsonParseError json_error;
	QJsonDocument jsonDoc(QJsonDocument::fromJson(message, &json_error));
	if (json_error.error == QJsonParseError::NoError)
	{
		if (jsonDoc.isObject())
		{
			QJsonObject obj = jsonDoc.object();
			if (obj.contains(node))
			{
				if (subnode != NULL)
				{
					QJsonObject subObj = obj.value(node).toObject();
					return subObj.take(subnode);
				}
				return obj.value(node);
			}
			return QJsonValue();
		}
		return QJsonValue();
	}
	return QJsonValue();
}

QString iDevice::MakeJsonMessage(QString key, QJsonValue value)
{
	QJsonObject  parameterObject;
	parameterObject.insert(key, value);

	QJsonObject json;
	json.insert("id", 1);
	json.insert("method", "thing.event.property.post");
	json.insert("params", QJsonValue(parameterObject));

	QJsonDocument document;
	document.setObject(json);
	QByteArray byteArray = document.toJson(QJsonDocument::Compact);

	QString strJson(byteArray);

	return strJson;
}

void iDevice::OTA_Process()
{
	//get url version size	md5
}
void iDevice::PUB_tag_data(iTag* tag)
{
	//to publish the data of tag
	QMqttClient::ClientState status = client->state();
	if (status != QMqttClient::Connected) 
		return;

	//todo: for each tag , there is a property on server to save the temperature
	//QString MESSAGE_FORMAT = QString("{\"id\":3,\"params\":{\"IndoorTemperature\":%1},\"method\":\"thing.event.property.post\"}").arg(temp, 0, 'f', 1);
	QString MESSAGE_FORMAT = QString("{\"params\":{\"IndoorTemperature\":%1}}").arg(tag->T_temp, 0, 'f', 1);
	client->publish(PubParameterTopic, MESSAGE_FORMAT.toUtf8());
}
void iDevice::PUB_tag_event(iTag* tag)
{	
	if (tag->T_alarm_offline)
	{
		//to publish the event

		tag->T_alarm_offline = false;
	}
}
void iDevice::PUB_rdm_data()
{
	//to publish rdm data

}
void iDevice::PUB_rdm_event()
{
	//to publish rdm event
	if (RDM->RDM_alarm)
	{
		RDM->RDM_alarm = false;
	}
}