#include "Model.h"
#include "irdm.h"
#include <QLineEdit>
#include <QIntValidator>


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

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		iRdm *rdm = listRdm.at(index.row());

		switch (index.column()) {
		case _Model::Name:
			return rdm->m_name;

		case _Model::IP:
			return rdm->m_ip;

		case _Model::MAC:
			return rdm->m_MAC;

		case _Model::VERSION:
			return rdm->m_Version;

		default:
			return QVariant();
		}
	}
	else if (role == Qt::DecorationRole)
	{
		
	}
	else if (role == Qt::UserRole)
	{
		iRdm *rdm = listRdm.at(index.row());
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
		case _Model::VERSION:
			return QString::fromLocal8Bit("软件版本");
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
	iRdm *rdm = listRdm.at(row);
	if (!rdm)
		return false;

	if (role == Qt::DecorationRole)
	{

	}
	else if (role == Qt::EditRole)
	{
		if (index.column() == _Model::IP)
		{
			rdm->m_ip = value.toString();
			emit IpChanged(rdm);
		}
		return true;
	}
	return false;
}
bool RdmModel::insertmyrow(int row, iRdm * rdm)
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
		delete listRdm.at(row);
		listRdm.removeAt(row);	
	}

	endRemoveRows();
	return true;
}
Qt::ItemFlags RdmModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;

	if (index.column() == _Model::IP)
		return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
	else
		return QAbstractTableModel::flags(index);
}
//--------------------------------------------------------------------------------------------------
//Tag Model 
//--------------------------------------------------------------------------------------------------
TagModel::TagModel(QObject *parent)
	: QAbstractTableModel(parent)
{
	editColumns = 0;
}

int TagModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return listTags.count();
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

	if (index.row() >= listTags.count() || index.row() < 0)
		return QVariant();

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		iTag *tag = listTags.at(index.row());

		switch (index.column()) 
		{
		case _Model::SID:
			return tag->t_sid;
		case _Model::UID:
			return tag->t_uid;
		case _Model::EPC:
			return tag->t_epc;
		case _Model::TEMP:
			return tag->t_online == 1? QString::number(tag->t_temperature,'f',1) : "--.-";
		case _Model::ALARM:
			return tag->t_alarm == 1? QString::fromLocal8Bit("是") : QString::fromLocal8Bit("否");
		case _Model::UPLIMIT:
			return tag->t_uplimit;
		case _Model::RSSI:
			return tag->t_rssi;
		case _Model::OCRSSI:
			return tag->t_oc_rssi;
		case _Model::NOTE:
			return tag->t_note;
		default:
			return QVariant();
		}
	}
	else if (role == Qt::DecorationRole)
	{

	}
	else if (role == Qt::UserRole)
	{
		iTag *tag = listTags.at(index.row());
		if (tag) 
			return (uint)tag;
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
			return QString::fromLocal8Bit("序号(SID)");
		case _Model::UID:
			return QString::fromLocal8Bit("识别号(UID)");
		case _Model::EPC:
			return QString::fromLocal8Bit("名称(EPC)");
		case _Model::TEMP:
			return QString::fromLocal8Bit("温度");
		case _Model::ALARM:
			return QString::fromLocal8Bit("报警");
		case _Model::UPLIMIT:
			return QString::fromLocal8Bit("报警温度");
		case _Model::RSSI:
			return QString::fromLocal8Bit("反射信号强度");
		case _Model::OCRSSI:
			return QString::fromLocal8Bit("接受信号强度");
		case _Model::NOTE:
			return QString::fromLocal8Bit("备注");
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
	iTag *tag = listTags.at(row);
	if (!tag)
		return false;

	if (role == Qt::DisplayRole)
	{

	}
	else if (role == Qt::EditRole)
	{
		if (index.column() == _Model::EPC)
		{
			if (value.toString().count() > 10 || value.toString() == tag->t_epc)
				return false;
			QString epc;
			if (value.toString().count() % 2 != 0)
				epc = value.toString() + QChar(' ');
			else
				epc = value.toString();
			if (epc.count() == 0 )
			{
				emit dataFailed(index, QString::fromLocal8Bit("更改失败：标签名称不能为空."));			//change epc failed
				return false;
			}
			else if (hasTag(epc))
			{
				emit dataFailed(index, QString::fromLocal8Bit("更改失败：标签名称已经存在."));			//change epc failed
				return false;
			}
			else
				tag->t_epc = epc;
		}
		else if (index.column() == _Model::UPLIMIT)
		{
			if (value.toInt() < 0 || value.toInt() >= 100)											//data validation for uplimit
			{
				emit dataFailed(index, QString::fromLocal8Bit("更改失败：温度界限设置错误."));			//change uplimit failed
				return false;
			}
			tag->t_uplimit = value.toInt();
		}
		else if (index.column() == _Model::NOTE)
			tag->t_note = value.toString();

		emit dataChanged(index, index);
		return true;	
	}
	return false;
}
bool TagModel::removeRows(int row, int count, const QModelIndex & parent)
{
	Q_UNUSED(parent);
	beginRemoveRows(QModelIndex(), row, row + count - 1);

	for (int i = 0; i < count; i++)
	{
		delete listTags.at(row);
		listTags.removeAt(row);
	}

	endRemoveRows();
	return true;
}
bool TagModel::insertRow(int row, iTag * tag)
{
	beginInsertRows(QModelIndex(), row, row);
	listTags.insert(row, tag);
	endInsertRows();
	return true;
}
Qt::ItemFlags TagModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;

	if ((1 << index.column()) & editColumns)
		return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
	else
		return QAbstractTableModel::flags(index);
}
bool TagModel::hasTag(const QString& epc)
{
	for (iTag *tag : listTags)
	{
		if (tag->t_epc == epc)
			return true;
	}
	return false;
}
bool TagModel::hasTag(quint64 uid, const QString& epc)
{ 
	for (iTag *tag : listTags)
	{
		if (tag->t_uid == uid || tag->t_epc == epc)
			return true;
	}
	return false;
}
void TagModel::setTagEpc(quint64 uid, const QString& epc)
{
	for (int i = 0; i < listTags.count(); i++)
	{
		if (listTags[i]->t_uid == uid)
		{
			setData(index(i, _Model::EPC), epc,Qt::EditRole);
			return;
		}
	}
}
//IP address edit delegate
IpAddressDelegate::IpAddressDelegate(QObject *parent)
	: QStyledItemDelegate(parent)
{
}
QWidget *IpAddressDelegate::createEditor(QWidget *parent,const QStyleOptionViewItem & option,const QModelIndex & index) const
{
	QLineEdit *editor = new QLineEdit(parent);
	QRegExp regExp("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-4]|[01]?\\d\\d?)");
	editor->setValidator(new QRegExpValidator(regExp, parent));

	return editor;
}
void IpAddressDelegate::setEditorData(QWidget *editor,const QModelIndex &index) const
{
	QString value = index.model()->data(index, Qt::EditRole).toString();

	QLineEdit *lineedit = static_cast<QLineEdit*>(editor);
	lineedit->setText(value);
}
void IpAddressDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,const QModelIndex &index) const
{
	QLineEdit *lineedit = static_cast<QLineEdit*>(editor);

	QString  value = lineedit->text();

	model->setData(index, value, Qt::EditRole);
}
void IpAddressDelegate::updateEditorGeometry(QWidget *editor,const QStyleOptionViewItem &option, const QModelIndex & index) const
{
	editor->setGeometry(option.rect);
}

//EPC edit delegate
LengthLimitDelegate::LengthLimitDelegate(int length, bool supportchinese,QObject *parent)
	: QStyledItemDelegate(parent)
{
	m_limit = length;
	m_chinese = supportchinese;
}
void LengthLimitDelegate::ontextchanged(QString text)
{
	QLineEdit *edit = qobject_cast<QLineEdit*>(sender());

	int bytes = text.toLocal8Bit().length();
	if (bytes > m_limit)
	{
		while(text.toLocal8Bit().length()>m_limit)
			text.chop(1);

		edit->setText(text);
	}
}
QWidget *LengthLimitDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	QLineEdit *editor = new QLineEdit(parent);
	if (m_chinese)
	{
		connect(editor, &QLineEdit::textChanged, this, &LengthLimitDelegate::ontextchanged);
	}
	else
	{
		editor->setMaxLength(m_limit);
		QRegExp regExp("^[^\u4e00-\u9fa5]{0,}$");		// do not input chinese
		editor->setValidator(new QRegExpValidator(regExp, parent));
	}
	return editor;
}
void LengthLimitDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	QString value = index.model()->data(index, Qt::EditRole).toString();
	QLineEdit *lineedit = static_cast<QLineEdit*>(editor);
	lineedit->setText(value);
}
void LengthLimitDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	QLineEdit *lineedit = static_cast<QLineEdit*>(editor);
	QString  value = lineedit->text();
	model->setData(index, value, Qt::EditRole);
}
void LengthLimitDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & index) const
{
	editor->setGeometry(option.rect);
}
// RangeLimit for temperature limit input
RangeLimitDelegate::RangeLimitDelegate(int min, int max,QObject *parent)
	: QStyledItemDelegate(parent)
{
	m_min = min;
	m_max = max;
}
QWidget *RangeLimitDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	QLineEdit *editor = new QLineEdit(parent);
	editor->setValidator(new QIntValidator(m_min, m_max));
	return editor;
}
void RangeLimitDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	QString value = index.model()->data(index, Qt::EditRole).toString();
	QLineEdit *lineedit = static_cast<QLineEdit*>(editor);
	lineedit->setText(value);
}
void RangeLimitDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	QLineEdit *lineedit = static_cast<QLineEdit*>(editor);
	QString  value = lineedit->text();
	model->setData(index, value, Qt::EditRole);
}
void RangeLimitDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & index) const
{
	editor->setGeometry(option.rect);
}