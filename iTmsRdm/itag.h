#pragma once

#include <QObject>

class iTag : public QObject
{
	Q_OBJECT

public:
	iTag(quint64 tag_ID, QObject *parent);
	~iTag();
};
