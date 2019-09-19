#pragma once

#include <QObject>

enum LED {
	LED_STATUS = 0,
	LED_ALARM,
};

#define LED1			19			//green(HW_V2)
#define LED2			26			//yellow(HW_V2),green(HW_V1)

#define IO_IN			0
#define IO_OUT			1

class iLed : public QObject
{
	Q_OBJECT

public:
	iLed(int hwVer,QObject *parent);
	~iLed();

#ifdef __linux__
	void setLed(int pin, bool on);
#endif
	void toggleled(int led);

private:
	bool m_alarmledOn;
	bool m_statusledOn;
	int	 m_StatusLedPin;
	int	 m_AlarmLedPin;

#ifdef __linux__
	void gpioExport(int pin);
	void gpioUnexport(int pin);
	void gpioDirection(int pin, int direction);
#endif
};
