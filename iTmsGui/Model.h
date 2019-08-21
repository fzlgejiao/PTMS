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
		ALARM,
		UPLIMIT,
		RSSI,
		OCRSSI,
		NOTE,
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
	Qt::ItemFlags flags(const QModelIndex &index) const;
	
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
	bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());
	Qt::ItemFlags flags(const QModelIndex &index) const;
	
	bool insertRow(int row, iTag * tag);

	bool hasTag(quint64 uid);
	void setEditColumns(int columns) { editColumns |= columns; }
	void setTagEpc(quint64 uid, const QString& epc);

	QList<iTag * >	& taglist() { return listTags; }


private:
	QList<iTag *>	listTags;
	int				editColumns;

signals:
	void RdmModified(bool);
};


