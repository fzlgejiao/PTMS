#pragma once

#include <QObject>

class iRDM;
class iDevice : public QObject
{
	Q_OBJECT

public:
	iDevice(QObject *parent);
	~iDevice();

private:
	iRDM*		RDM;
};
