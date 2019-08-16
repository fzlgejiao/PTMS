#include "iLed.h"

iLed::iLed(QObject *parent)
	: QObject(parent)
{
#ifdef __linux__
	gpioExport(AlarmLed);
	gpioExport(StatusLed);

	gpioDirection(AlarmLed, Output);
	gpioDirection(StatusLed, Output);

	setLed(AlarmLed, false);
	setLed(StatusLed, false);
#endif
	m_alarmledOn = false;
	m_statusledOn = false;
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

void iLed::toggleled(int pin)
{	
	if (pin == AlarmLed)
	{
		m_alarmledOn = !m_alarmledOn;
#ifdef __linux__	
		setLed(pin, m_alarmledOn);
#endif
	}
	else if (pin == StatusLed)
	{
		m_statusledOn = !m_statusledOn;
#ifdef __linux__	
		setLed(pin, m_statusledOn);
#endif
	}
}