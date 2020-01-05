#include "cConfigdata.h"
#include <stdio.h>
#include <string.h>
#include <string>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#define CONF_PATH "/etc/secure/security.conf"
#define CONF_PATH_APP "/etc/secure/security_app.conf"
using namespace std;

void cConfigdata::vReadConfigData()
{

	char buffer[100];
	FILE *file = fopen ( CONF_PATH, "r" );
    ssize_t read;
	char* line = NULL;

	size_t len = 0;
	char c;
	string str;
	string str2;
	int pos1=0,pos2 =0;


	if(file == NULL)
	{
		//vSetDefaultConfiguration();
		printf("\nConfigFileNotAvailable");
		exit(0);
	}

	while ((read = getline(&line, &len, file)) != -1)
	{

			//parse the line
			str.assign(line, strlen(line));
			if(str.find("LIGHT :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1,(pos2-pos1-1));
				m_lightgpio=atoi(str2.c_str());
			}
			else if(str.find("TIMEOUT :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1, (pos2-pos1-1));
				m_timeout=atoi(str2.c_str());
			}
			else if(str.find("MODE :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1, (pos2-pos1-1));
				m_mode=atoi(str2.c_str());
			}

			else if(str.find("BUZZER :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1,(pos2-pos1-1));
				m_buzzergpio=atoi(str2.c_str());
			}
			else if(str.find ("OBJECT :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1,(pos2-pos1-1));
				m_gpioconfig[m_numberofgpio++]=atoi(str2.c_str());
			}
			else if(str.find ("CAM_PORT :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1,(pos2-pos1-1));
				m_port = atoi(str2.c_str());
			}
            else if(str.find ("MAX_FILES :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1,(pos2-pos1-1));
				m_maxfiles = atoi(str2.c_str());
			}
			else if(str.find ("RECORD :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1,(pos2-pos1-1));
				m_file_loc = new char[str2.length()+1];
				strcpy(m_file_loc,str2.c_str());
			}
			else if(str.find ("NET_LOC :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1,(pos2-pos1-1));
				m_network_file_loc = new char[str2.length()+1];
				strcpy(m_network_file_loc,str2.c_str());
			}
            else if(str.find ("CONN :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1,(pos2-pos1-1));
				m_interface = new char[str2.length()+1];
				strcpy(m_interface,str2.c_str());
			}
            else if(str.find ("BUZZER_STATE :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1,(pos2-pos1-1));
				m_buzzerstate = atoi(str2.c_str());
			}
			else if(str.find ("LIGHT_STATE :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1,(pos2-pos1-1));
				m_lightstate = atoi(str2.c_str());
			}
            else if(str.find ("FRAMERATE :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1,(pos2-pos1-1));
				m_framerate = atoi(str2.c_str());

			}
            else if(str.find ("BITRATE :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1,(pos2-pos1-1));
				m_bitrate = atoi(str2.c_str());

			}
            else if(str.find ("HEIGHT :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1,(pos2-pos1-1));
				m_height = atoi(str2.c_str());

			}
            else if(str.find ("WIDTH :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1,(pos2-pos1-1));
				m_width = atoi(str2.c_str());

			}
            else if(str.find ("WATCHDOG :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1,(pos2-pos1-1));
				m_watchdog = bool(atoi(str2.c_str()));

			}
            else if(str.find ("SMS :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1,(pos2-pos1-1));
				m_sms = bool(atoi(str2.c_str()));

			}
            else if(str.find ("CAMS :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1,(pos2-pos1-1));
				m_cams = atoi(str2.c_str());

			}
            else if(str.find ("PORT2 :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1,(pos2-pos1-1));
				m_port2 = atoi(str2.c_str());

			}
            else if(str.find ("PORT3 :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1,(pos2-pos1-1));
				m_port3 = atoi(str2.c_str());

			}
            else if(str.find ("PORT4 :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1,(pos2-pos1-1));
				m_port4 = atoi(str2.c_str());

			}
            else if(str.find ("COMM_PORT :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1,(pos2-pos1-1));
				m_commport = atoi(str2.c_str());

			}


			free(line);
			line=NULL;
			len=0;
       }
       iNetworkAddress();

       fclose(file);
}


void cConfigdata::vReadConfigDataFromApp()
{

	char buffer[100];
	FILE *file = fopen ( CONF_PATH_APP, "r" );
    ssize_t read;
	char* line = NULL;

	size_t len = 0;
	char c;
	string str;
	string str2;
	int pos1=0,pos2 =0;


	if(file == NULL)
	{
		//vSetDefaultConfiguration();
		printf("\nConfigFileNotAvailable");
		//exit(0);
		return;
	}

	while ((read = getline(&line, &len, file)) != -1)
	{

			//parse the line
			str.assign(line, strlen(line));
			if(str.find("TIMEOUT :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1, (pos2-pos1-1));
				m_timeout=atoi(str2.c_str());
			}
			else if(str.find("COMM_PORT :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1, (pos2-pos1-1));
				m_commport=atoi(str2.c_str());
			}

            else if(str.find ("BUZZER_STATE :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1,(pos2-pos1-1));
				m_buzzerstate = atoi(str2.c_str());
			}
			else if(str.find ("LIGHT_STATE :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1,(pos2-pos1-1));
				m_lightstate = atoi(str2.c_str());
			}

            else if(str.find ("SMS :") != -1)
			{
				pos1 = str.find("[");
				pos2 = str.find("]");
				str2 = str.substr(pos1+1,(pos2-pos1-1));
				m_sms = bool(atoi(str2.c_str()));

			}

			free(line);
			line=NULL;
			len=0;
        }

       fclose(file);
}

void cConfigdata::vWriteData_App()
{
  string input;
  ofstream out(CONF_PATH_APP);

  out << "TIMEOUT :[" <<m_timeout<<"]"<<endl;
  out << "COMM_PORT :[" <<m_commport<<"]"<<endl;

  out << "BUZZER_STATE :[" <<m_buzzerstate<<"]"<<endl;
  out << "LIGHT_STATE :[" <<m_lightstate<<"]"<<endl;
  out << "SMS :[" <<m_sms<<"]"<<endl;
  out.close();



}

int cConfigdata::iGetObsGPIOConfig(int* gpio_ports)
{
	for(int i=0; i<m_numberofgpio; i++)
	{
		gpio_ports[i]=m_gpioconfig[i];
	}

	return m_numberofgpio;
}
int cConfigdata::iGetTimeout()
{
	return m_timeout;
}

int cConfigdata::iGetMode()
{
	return m_mode;
}


int cConfigdata::iGetLightSensorGPIO()
{
	return m_lightgpio;
}

int cConfigdata::iGetBuzzerSensorGPIO()
{
	return m_buzzergpio;
}

cConfigdata::cConfigdata()
{
	m_lightgpio = 0; // default port
	m_gpioconfig[0]=0;
	m_numberofgpio = 0;
	m_mode=PLAY_NETWORK_MODE; //network mode is default
	m_port=2047; //local port
	m_network_file_loc = NULL;
	m_file_loc = NULL;
	m_interface = NULL;
	m_maxfiles = 5;
	m_framerate=5;
	m_bitrate=1000000;
	m_height=480;
	m_width=640;
	m_lightstate=LIGHT_STATE_NIGHT;
	m_buzzerstate=BUZZER_STATE_NIGHT;
	m_watchdog=true;
    m_sms=true;
    m_commport=2040;
    m_cams=1;
    m_port2=0;
    m_port3=0;
    m_port4=0;
}

cConfigdata::~cConfigdata()
{
}

char* cConfigdata::pGetIPAddress()
{
	return m_ip_address;
}

int cConfigdata::iGetPort()
{
	return m_port;
}

int cConfigdata::iGetMaxFiles()
{
	return m_maxfiles;
}

 char* cConfigdata::pGetFileLocation()
{
	return m_file_loc;
}
 char* cConfigdata::pGetNetworkFileLocation()
{
	return m_network_file_loc;
}

int cConfigdata::iNetworkAddress()
{

    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;

    /* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, m_interface, IFNAMSIZ-1);

    ioctl(fd, SIOCGIFADDR, &ifr);

    close(fd);

    sprintf(m_ip_address,"%s",inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

    printf("Address is %s", m_ip_address);

    if(strcmp(m_ip_address, "0.0.0.0") == 0 || strcmp(m_ip_address, "") == 0)
    {
        return 0;
    }

    return 1;
}

int cConfigdata::iGetBuzzerState()
{
    return m_buzzerstate;
}

int cConfigdata::iGetLightState()
{
    return m_lightstate;
}


int cConfigdata::iGetFrameRate()
{
    return m_framerate;
}
int cConfigdata::iGetWidth()
{
    return m_width;
}
int cConfigdata::iGetHeight()
{
    return m_height;
}
int cConfigdata::iGetBitrate()
{
    return m_bitrate;

}
int cConfigdata::iGetcams()
{
    return m_cams;

}
int cConfigdata::iGetport2()
{
    return m_port2;

}
int cConfigdata::iGetport3()
{
    return m_port3;

}
int cConfigdata::iGetport4()
{
    return m_port4;

}
bool cConfigdata::bGetWatchdog()
{
    return m_watchdog;

}
bool cConfigdata::bGetSMS()
{
    return m_sms;
}

void cConfigdata::vSetCommport(int port)
{
    m_commport=port;

}

int cConfigdata::iGetcommport()
{
    return m_commport;

}
void cConfigdata::vSetTimeout(int timeout)
{
    m_timeout = timeout;
}

void cConfigdata::vSetLightState(int state)
{
    m_lightstate = state;
}

void cConfigdata::vSetBuzzerState(int state)
{
    m_buzzerstate = state;
}

void cConfigdata::vSetSMSState(int state)
{
    m_sms = state;
}


/* text file format
OBJECT : [22]
LIGHT : [16]
BUZZER : [20]
*/


