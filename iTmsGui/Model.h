#pragma once

#include <QList>
#include <QAbstractTableModel>


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

	

private:
	QList<QString>	listName;													
	QList<QString>	listIp;												
	QList<QString>	listMAC;
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
};


