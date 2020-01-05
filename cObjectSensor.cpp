#include "cObjectSensor.h"
#include "cMessageQueue.h"
#include <unistd.h>
#include <sys/poll.h>
cObjectSensor* tempsensor=NULL; //temporary hack

void cObjectSensor::vInitGPIO()
{
    int fd;
	char buf[50];
	char read_value;

	//exporting gpio pin
	fd = open("/sys/class/gpio/export", O_WRONLY);
	sprintf(buf, "%d", m_GPIONumber);
	write(fd, buf, strlen(buf));
	close(fd);

	//write the direction now
	sprintf(buf, "/sys/class/gpio/gpio%d/direction", m_GPIONumber);
	fd = open(buf, O_WRONLY);
	write(fd, "in", 2);
	close(fd);
}
void cObjectSensor::vReadGPIO()
{
    int fd;
	char buf[50];
	char read_value;
	//now read the gpio pin
	sprintf(buf, "/sys/class/gpio/gpio%d/value", m_GPIONumber);

	/* payload for sending data */
	char send_data[3];
	send_data[0] = OBJECTSENSOR_MSG;
	send_data[2]='\0';

	fd=open(buf,O_RDONLY);
    while(m_close)
    {
	//fd=open(buf, O_RDONLY);
		lseek(fd, 0, SEEK_SET);
		read(fd, &read_value, 1);

		if(read_value == '1')
		{

			if(m_bCurrentState == false)
			{
				//change seen..so post message
				m_bCurrentState = true;
				send_data[1]=true;
				cMessageQueue::vPostMessage(send_data);
			}
		}
		else if(read_value == '0')
		{
			if(m_bCurrentState == true)
			{
				//change seen..so post message
				m_bCurrentState = false;
				send_data[1]=false;
				cMessageQueue::vPostMessage(send_data);
			}
		}
		//close(fd);
	};

	close(fd);
	m_close = true;
}

void* cObjectSensor::vCreateGPIO(void* data) //thread entry function
{
	cObjectSensor* pObjectSensor = (cObjectSensor*)data /*threaddata->pObjectSensor*/;

	//calling a loop to read GPIO data
	pObjectSensor->vInitGPIO();
	pObjectSensor->vReadGPIO();
	return NULL;
}


/* constructor and destructor functions */
cObjectSensor::cObjectSensor(int gpio_pin)
{
	m_bCurrentState=true;
	m_GPIONumber = gpio_pin;
	m_close = true;
}

void cObjectSensor::vCloseRead()
{
    m_close=false;
     int fd;
	char buf[50];
	char read_value;

	//exporting gpio pin
	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	sprintf(buf, "%d", m_GPIONumber);
	write(fd, buf, strlen(buf));
	close(fd);


}

