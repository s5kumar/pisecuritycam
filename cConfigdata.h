#include "common_header.h"
#include <stdio.h>


#define MAX_GPIO 4
#ifndef CCONFIGDATA_H_
#define CCONFIGDATA_H_
class cConfigdata
{
	public:

		void vReadConfigData();
		void vReadConfigDataFromApp();
		void vWriteData_App();

		cConfigdata(); //constructor

		~cConfigdata(); //destructor


	public: //data parts
		int iGetObsGPIOConfig(int* gpio_ports); //returns number of object sensors

		int iGetLightSensorGPIO(); //returns light sensor gpio port

		int iGetBuzzerSensorGPIO(); //returns light sensor gpio port

		int iNetworkAddress();

		char* pGetIPAddress();

		char* pGetFileLocation();

		char* pGetNetworkFileLocation();

		int iGetPort();

		int iGetTimeout();

		int iGetcommport();

		int iGetMode();

		int iGetMaxFiles();

		int iGetBuzzerState();

		int iGetLightState();

		int iGetFrameRate();

		int iGetWidth();

		int iGetHeight();

		int iGetBitrate();

		int iGetcams();

		int iGetport2();

		int iGetport3();

		int iGetport4();

		bool bGetWatchdog();

		bool bGetSMS();

		void vSetCommport(int port);

		void vSetTimeout(int timeout);

		void vSetLightState(int state);

		void vSetBuzzerState(int state);

		void vSetSMSState(int state);

	private: //data
		int m_gpioconfig[MAX_GPIO];

		int m_lightgpio;

		int m_commport;

		int m_buzzergpio;

		int m_numberofgpio;

		char m_ip_address[20];

		char* m_file_loc;

		char* m_network_file_loc;

		char* m_interface;

		int m_port;

		int m_timeout;

		int m_mode;

        int m_maxfiles;

        int m_buzzerstate;

		int m_lightstate;

        int m_framerate;

        int m_cams;

        int m_port2;

        int m_port3;

        int m_port4;

        int m_height;

        int m_width;

        int m_bitrate;

        bool m_watchdog;

        bool m_sms;

};
#endif //COBJECTSENSOR_H_
