#include "iLed.h"
#include "irdm.h"
#ifdef __linux__
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
iLed::iLed(int hwVer,QObject *parent)
	: QObject(parent)
{
#ifdef __linux__
	gpioExport(LED1);
	gpioExport(LED2);

	gpioDirection(LED1, IO_OUT);
	gpioDirection(LED2, IO_OUT);

	setLed(LED1, false);
	setLed(LED2, false);
#endif
	m_alarmledOn = false;
	m_statusledOn = false;
	if (hwVer == HW_V1)
	{
		m_StatusLedPin = LED2;
		m_AlarmLedPin = 0;
	}
	else
	{
		m_StatusLedPin = LED1;
		m_AlarmLedPin = LED2;
	}
}

iLed::~iLed()
{
}

#ifdef __linux__
void iLed::gpioExport(int pin)
{
	int fd;
	char buf[255];
	fd = open("/sys/class/gpio/export", O_WRONLY);
	sprintf(buf, "%d", pin);
	write(fd, buf, strlen(buf));
	::close(fd);									
}
void iLed::gpioUnexport(int pin)
{
	int fd;
	char buf[255];
	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	sprintf(buf, "%d", pin);
	write(fd, buf, strlen(buf));
	::close(fd);
}
void iLed::gpioDirection(int pin, int direction) 
{
	int fd;
	char buf[255];
	sprintf(buf, "/sys/class/gpio/gpio%d/direction", pin);
	fd = open(buf, O_WRONLY);

	if (direction)
	{
		write(fd, "out", 3);
	}
	else
	{
		write(fd, "in", 2);
	}
	::close(fd);
}
void iLed::setLed(int pin, bool on)
{
	int fd;
	char buf[255];
	sprintf(buf, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(buf, O_WRONLY);
	if(on)
		write(fd, "1", 1);		
	else
		write(fd, "0", 1);

	::close(fd);
}
#endif

void iLed::toggleled(int led)
{	
	if (led == LED_ALARM)
	{
		m_alarmledOn = !m_alarmledOn;
#ifdef __linux__	
		setLed(m_AlarmLedPin, m_alarmledOn);
#endif
	}
	else if (led == LED_STATUS)
	{
		m_statusledOn = !m_statusledOn;
#ifdef __linux__	
		setLed(m_StatusLedPin, m_statusledOn);
#endif
	}
}