#include "irdm.h"

iRdm::iRdm(const QString &name, const QString &ip, const QString& mac, const QString& version, QObject *parent)
	: QObject(parent)
{
	m_name = name;
	m_ip = ip;
	m_MAC = mac;
	m_Version = version;
}

iRdm::~iRdm()
{
}

iTag::iTag(QObject *parent)
	: QObject(parent)
{

}
iTag::~iTag()
{
}