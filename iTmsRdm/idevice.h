#pragma once

#include <QObject>

class iDevice : public QObject
{
	Q_OBJECT

public:
	iDevice(QObject *parent);
	~iDevice();
};
