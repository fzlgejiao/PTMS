#include "irdm.h"
#include "Model.h"
#include <QSqlQuery> 
#include <QSqlError>
#include <QTimerEvent>
#include <QDateTime>

iRdm::iRdm(const QString &name, const QString &ip, const QString& mac, const QString& version, const QString& note, QObject *parent)
	: QObject(parent)
{
	m_name = name;
	m_ip = ip;
	m_MAC = mac;
	m_Version = version;
	m_note = note;
	m_comname = "";
	m_bModified = false;
}

iRdm::~iRdm()
{
}



iTag::iTag(quint64 uid, const QString& epc,QObject *parent)
	: QObject(parent),t_uid(uid),t_epc(epc)
{
	t_sid = 0;
	t_rssi = 0;
	t_oc_rssi = 0;
	t_temperature = 0.;
	t_uplimit = 70;
	t_alarm = 0;
}
iTag::iTag(const iTag& tag)
{
	t_sid = tag.t_sid;
	t_uid = tag.t_uid;
	t_epc = tag.t_epc;
	t_rssi = tag.t_rssi;
	t_oc_rssi = tag.t_oc_rssi;
	t_temperature = tag.t_temperature;
	t_note = tag.t_note;
	t_uplimit = tag.t_uplimit;
	t_alarm = tag.t_alarm;
}
iTag::~iTag()
{
}


bool iSys::DB_save_rdm(iRdm *rdm)
{
	QSqlQuery	query;
	//check if rdm exists
	if (query.exec(QString("SELECT * FROM RDMS WHERE ID='%1'").arg(rdm->m_MAC)))
	{
		if (query.next())
		{
			if (query.exec(QString("UPDATE RDMS SET IP='%1',NAME='%2',VERSW='%3',DESC='%4' WHERE ID='%5'").arg(rdm->m_ip).arg(rdm->m_name).arg(rdm->m_Version).arg(rdm->m_note).arg(rdm->m_MAC)))
				return true;
		}
		else
		{
			if (query.exec(QString("INSERT INTO RDMS (ID, IP, NAME, VERSW, DESC) VALUES('%1','%2','%3','%4','%5')")
				.arg(rdm->m_MAC)
				.arg(rdm->m_ip)
				.arg(rdm->m_name)
				.arg(rdm->m_Version)
				.arg(rdm->m_note)))
				return true;
		}
	}

	//QString err = query.lastError().text();
	return false;
}
bool iSys::DB_save_tags(iRdm* rdm)
{
	QSqlDatabase db = QSqlDatabase::database();
	QSqlQuery	query;
	db.transaction();																				//start transaction to make db op fast

	//insert tag into db
	if (false == query.prepare("INSERT INTO TAGS (TIME, TID, SID, NAME, TEMP, ALARM, RSSI, OCRSSI,RDMID) "
		"VALUES (:TIME, :TID, :SID, :NAME, :TEMP, :ALARM, :RSSI, :OCRSSI, :RDMID)"))
		return false;

	QDateTime datetime = QDateTime::currentDateTime();
	for (iTag *tag : tagModelData->taglist())
	{
		datetime = datetime.addMSecs(1);
		QString time = datetime.toString("yyyy-MM-dd   hh:mm:ss.zzz");
		query.bindValue(":TIME", time);
		query.bindValue(":TID", tag->t_uid);
		query.bindValue(":SID", tag->t_sid);
		query.bindValue(":NAME", tag->t_epc);
		query.bindValue(":TEMP", tag->t_temperature);
		query.bindValue(":ALARM", tag->t_alarm);
		query.bindValue(":RSSI", tag->t_rssi);
		query.bindValue(":OCRSSI", tag->t_oc_rssi);
		query.bindValue(":RDMID", rdm->m_MAC);
		query.exec();
	}

	return db.commit();
}
void iSys::DB_clear()
{
	QSqlQuery	query;
	query.exec("DELETE FROM TAGS");
}