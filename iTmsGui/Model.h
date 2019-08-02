#pragma once

#include <QList>
#include <QAbstractTableModel>

namespace _Model {

	typedef enum
	{
		Name = 0,
		IP,
		MAC,
		RdmModelColumnCnt
	}RdmModelColumns;

	typedef enum
	{
		SID = 0,
		UID,
		EPC,
		TEMP,
		UPLIMIT,
		RSSI,
		OCRSSI,
		TagModelColumnCnt
	}TagModelColumns;
}

class CTag :public QObject
{
	Q_OBJECT

public:
	CTag(QObject *parent = 0);
	~CTag();

private:
	friend class CRdm;

	int sid;
	quint64 UID;
	QString epc;
	qint8 rssi;
	quint8 oc_rssi;
	float temperature;
};

class CRdm :public QObject
{
	Q_OBJECT

public:
	CRdm(QString &name, QString &ip, QString& mac,QObject *parent = 0);
	~CRdm();

private:
	friend class RdmModel;
	friend class iRdmView;

	QString m_name;
	QString m_ip;
	QString m_MAC;
	
	QMap<quint64, CTag *> readabletaglist;
	QMap<quint64, CTag *> unassignedtaglist;
};



class RdmModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	RdmModel(QObject *parent = 0);

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;

	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

	bool insertmyrow(int row, CRdm * rdm);
	bool removeRows(int row, int count, const QModelIndex & parent);
	
	inline void clear() { listRdm.clear(); }

private:	
	QList<CRdm *>	listRdm;
};

class TagModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	TagModel(QObject *parent = 0);

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;

	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	
private:
	QList<QString>	listEpc;
	QList<int>		listRSSI;
	QList<int>		listOC_RSSI;
	QList<int>		listUplimit;
	QList<float>	listTemperature;
	QList<CTag *>	listTags;
};


