#include "Model.h"

CRdm::CRdm(QString &name, QString &ip, QString& mac,QObject *parent)
	: QObject(parent)
{
	m_name = name;
	m_ip = ip;
	m_MAC = mac;
}

CRdm::~CRdm()
{
}


//Rdm model
RdmModel::RdmModel(QObject *parent)
	: QAbstractTableModel(parent)
{	
}

int RdmModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return listRdm.count();
}

int RdmModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return _Model::RdmModelColumnCnt;
}

QVariant RdmModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (index.row() >= listRdm.count() || index.row() < 0)
		return QVariant();

	if (role == Qt::DisplayRole)
	{
		CRdm *rdm = listRdm.at(index.row());

		switch (index.column()) {
		case _Model::Name:
			return rdm->m_name;

		case _Model::IP:
			return rdm->m_ip;

		case _Model::MAC:
			return rdm->m_MAC;

		default:
			return QVariant();
		}
	}
	else if (role == Qt::DecorationRole)
	{
		
	}
	else if (role == Qt::UserRole)
	{
		CRdm *rdm = listRdm.at(index.row());
		if (rdm) return (uint)rdm;
	}
	return QVariant();
}

QVariant RdmModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Horizontal) {
		switch (section) {		
		case _Model::Name:
			return QString::fromLocal8Bit("设备名称");
		case _Model::IP:
			return QString::fromLocal8Bit("IP地址");
		case _Model::MAC:
			return QString::fromLocal8Bit("MAC地址");

		default:
			return QVariant();
		}
	}
	return QVariant();
}
bool RdmModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid())
		return false;
	int row = index.row();

	if (role == Qt::DecorationRole)
	{

	}
	return false;
}
bool RdmModel::insertmyrow(int row, CRdm * rdm)
{
	beginInsertRows(QModelIndex(), row, row);
	listRdm.insert(row, rdm);
	endInsertRows();
	return true;
}
bool RdmModel::removeRows(int row, int count, const QModelIndex & parent)
{
	Q_UNUSED(parent);
	beginRemoveRows(QModelIndex(), row, row + count - 1);

	for (int i = 0; i < count; i++)
	{
		listRdm.removeAt(row);	
	}

	endRemoveRows();
	return true;
}

//Tag Model 
TagModel::TagModel(QObject *parent)
	: QAbstractTableModel(parent)
{
}

int TagModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return listEpc.count();
}

int TagModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return _Model::TagModelColumnCnt;
}

QVariant TagModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (index.row() >= listEpc.count() || index.row() < 0)
		return QVariant();

	if (role == Qt::DisplayRole)
	{

	}
	else if (role == Qt::DecorationRole)
	{

	}
	else if (role == Qt::UserRole)
	{

	}
	return QVariant();
}

QVariant TagModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Horizontal) {
		switch (section) {
		case _Model::SID:
			return QString::fromLocal8Bit("序号");
		case _Model::UID:
			return QString::fromLocal8Bit("识别号");
		case _Model::EPC:
			return QString::fromLocal8Bit("名称");
		case _Model::TEMP:
			return QString::fromLocal8Bit("温度");
		case _Model::UPLIMIT:
			return QString::fromLocal8Bit("报警温度");
		case _Model::RSSI:
			return QString::fromLocal8Bit("反射信号强度");
		case _Model::OCRSSI:
			return QString::fromLocal8Bit("接受信号强度");
		default:
			return QVariant();
		}
	}
	return QVariant();
}
bool TagModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid())
		return false;
	int row = index.row();

	if (role == Qt::DisplayRole)
	{
		
	}
	return false;
}