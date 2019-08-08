#pragma once

#include <QList>
#include <QAbstractTableModel>

namespace _Model {

	typedef enum
	{
		Name = 0,
		IP,
		MAC,
		VERSION,
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

class iRdm;
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

	bool insertmyrow(int row, iRdm * rdm);
	bool removeRows(int row, int count, const QModelIndex & parent= QModelIndex());
	
	inline void clear() { listRdm.clear(); }

private:	
	QList<iRdm *>	listRdm;
};

class iTag;
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
	QList<iTag *>	listTags;
};


