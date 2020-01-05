#include "common_header.h"
#include <stdio.h>
#include <signal.h>
#include <time.h>

#ifndef COBJECTSENSOR_H_
#define COBJECTSENSOR_H_
class cObjectSensor
{
	public:
		cObjectSensor(int); //constructor

		~cObjectSensor();

		static void* vCreateGPIO(void* data);

		void vReadGPIO();

		cObjectSensor* m_pObjectSensor; //self pointer to be called from static function

		void vInitGPIO();

		void vCloseRead();

	private:
		bool m_bCurrentState;

		int m_GPIONumber;
		timer_t m_timerid;

		int m_fd;

		bool m_close;

};
#endif //COBJECTSENSOR_H_
