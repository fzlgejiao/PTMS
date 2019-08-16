#pragma once

#include <QObject>


#define AlarmLed		19			//red one
#define StatusLed		26			//yellow one

#define Input			0
#define Output			1

class iLed : public QObject
{
	Q_OBJECT

public:
	iLed(QObject *parent);
	~iLed();

#ifdef __linux__
	void setLed(int pin, bool on);
#endif
	void toggleled(int pin);

private:
	bool m_alarmledOn;
	bool m_statusledOn;

#ifdef __linux__
	void gpioExport(int pin);
	void gpioUnexport(int pin);
	void gpioDirection(int pin, int direction);
#endif
};
