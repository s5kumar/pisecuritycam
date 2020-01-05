/* Includes */
#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <pthread.h>    /* POSIX Threads */
#include <string.h>     /* String handling */
#include <gio/gio.h>
#include <glib-unix.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <string>
#include <signal.h>
#include <time.h>
#include <netdb.h>
#include <sstream>
#include <netinet/in.h>

#include "cStreamer.h"
#include "cObjectSensor.h"
#include "cMessageQueue.h"
#include "common_header.h"
#include "cConfigdata.h"
#include <iostream>
#include <ctime>

using namespace std;
//variable initialization
static cStreamer* m_pStreamer = NULL;
static cConfigdata g_configdata;
cObjectSensor* m_pObjectSensor[NUMBER_OBJECT_SENSOR];
int g_numofobjsensor = 0;
static bool g_IsTimerOn = false;
pthread_t objectsensor[NUMBER_OBJECT_SENSOR];
pthread_t sockcomm;
static int socks[100];
int num_clients = 0;
string version_num = "version:0.16:";
string init_time;

//timer defines

#define CLOCKID CLOCK_REALTIME
#define SIG SIGUSR1
timer_t timerid;

struct stStates
{
    int current_state;
    int pending_state;

};

void distributemessage(string str);
string currenttime()
{

  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];

  time (&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buffer,sizeof(buffer),"%d-%m-%Y %I:%M:%S",timeinfo);
  std::string str(buffer);

  return str;
}
string IntToString (int a)
{
    ostringstream temp;
    temp<<a;
    return temp.str();
}

static stStates stStateDef={MOTION_DETECTED,MOTION_DETECTED}; //default state

static void handler(int sig, siginfo_t *si, void *uc)
{
    char send_data[2];
	send_data[0] = TIMER_MSG;
	send_data[1]='\0';

    if(si->si_value.sival_ptr != &timerid){
        printf("Stray signal\n");
    } else {
        timer_delete(timerid);
        cMessageQueue::vPostMessage(send_data);
    }



}

void vCreateTimer()
{

    struct sigevent sev;
    struct itimerspec its;
    long long freq_nanosecs;
    sigset_t mask;
    struct sigaction sa;

    if(g_IsTimerOn)
    {
        timer_delete(timerid);
    }
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIG, &sa, NULL);

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIG;
    sev.sigev_value.sival_ptr = &timerid;
    timer_create(CLOCKID, &sev, &timerid);
    /* Start the timer */

    its.it_value.tv_sec = g_configdata.iGetTimeout() * 60;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;
    timer_settime(timerid, 0, &its, NULL);
    g_IsTimerOn = true;



}

void vInitBuzzerSensor()
{
	int fd;
	char buf[50];
	char read_value;

	//exporting gpio pin
	fd = open("/sys/class/gpio/export", O_WRONLY);

	sprintf(buf, "%d", g_configdata.iGetBuzzerSensorGPIO());

	write(fd, buf, strlen(buf));

	close(fd);
	//write the direction now
	sprintf(buf, "/sys/class/gpio/gpio%d/direction", g_configdata.iGetBuzzerSensorGPIO());

	fd = open(buf, O_WRONLY);

	write(fd, "out", 3);

	close(fd);

}

void vInitLightSensor()
{
	int fd;
	char buf[50];
	char read_value;

	//exporting gpio pin
	fd = open("/sys/class/gpio/export", O_WRONLY);

	sprintf(buf, "%d", g_configdata.iGetLightSensorGPIO());

	write(fd, buf, strlen(buf));

	close(fd);
	//write the direction now
	sprintf(buf, "/sys/class/gpio/gpio%d/direction", g_configdata.iGetLightSensorGPIO());

	fd = open(buf, O_WRONLY);

	write(fd, "out", 3);

	close(fd);

}

void vCreateComponents()
{
	//m_Configdata = new cConfigData();

	m_pStreamer = cStreamer::pGetInstance(g_configdata.pGetFileLocation(),g_configdata.pGetIPAddress(),g_configdata.iGetPort(), \
	g_configdata.iGetMode(), g_configdata.pGetNetworkFileLocation(), g_configdata.iGetMaxFiles(), g_configdata.iGetFrameRate(), \
	g_configdata.iGetBitrate(), g_configdata.iGetWidth(), g_configdata.iGetHeight());


	int objsensorconfig[10];
	int numofsensors = g_configdata.iGetObsGPIOConfig(objsensorconfig);

	//fill data for object sensor
	for(int i=0; i<numofsensors; i++)
	{
		m_pObjectSensor[i]=new cObjectSensor(objsensorconfig[i]);
	}
	g_numofobjsensor = numofsensors;

	vInitLightSensor();

	vInitBuzzerSensor();
}

void vWritetoLightSensor(bool bLight)
{
	int fd;
	char buf[50];
	time_t curtime;
	struct tm* loctime;
	curtime = time(NULL);
	loctime = localtime(&curtime);
	sprintf(buf, "/sys/class/gpio/gpio%d/value", g_configdata.iGetLightSensorGPIO());

	fd = open(buf,O_WRONLY);


	if( bLight && g_configdata.iGetLightState() == LIGHT_STATE_ON)
	{
		write(fd,"0",1); //switch on
	}
	else if(bLight && ((loctime->tm_hour < 6) || (loctime->tm_hour) > 17) && g_configdata.iGetLightState() == LIGHT_STATE_NIGHT)
	{
		write(fd,"0",1); //switch on
	}
	else
	{
		write(fd,"1",1); //switch off
	}

	close(fd);
}

void vWritetoBuzzerSensor(bool bBuzz)
{
	int fd;
	char buf[50];
	time_t curtime;
	struct tm* loctime;
	curtime = time(NULL);
	loctime = localtime(&curtime);

	sprintf(buf, "/sys/class/gpio/gpio%d/value", g_configdata.iGetBuzzerSensorGPIO());

	fd = open(buf, O_WRONLY);
	if( bBuzz && g_configdata.iGetBuzzerState() == BUZZER_STATE_ON)
	{

		write(fd,"1",1); //switch on
	}
	else if(bBuzz && !((loctime->tm_hour < 6) || (loctime->tm_hour > 17)) && g_configdata.iGetBuzzerState() == BUZZER_STATE_NIGHT)
	{

		write(fd,"1",1); //switch on
	}
	else if(bBuzz && g_configdata.iGetBuzzerState() == BUZZER_STATE_MOVE)
	{
		write(fd, "1", 1); //switch on
	}
	else
	{
		write(fd,"0",1); //switch off
	}
	close(fd);
}

void vJustWritetoLightSensor(int bLight)
{
	int fd;
	char buf[50];
    sprintf(buf, "/sys/class/gpio/gpio%d/value", g_configdata.iGetLightSensorGPIO());

	fd = open(buf,O_WRONLY);

    if(bLight == 1)
	{
	    printf("calling light 1\n");
		write(fd,"0",1); //switch on
	}
	else
    {
        write(fd,"1",1); //switch on
        printf("calling light 0\n");
    }

	close(fd);
}

void vJustWritetoBuzzerSensor(int bBuzz)
{
	int fd;
	char buf[50];

	sprintf(buf, "/sys/class/gpio/gpio%d/value", g_configdata.iGetBuzzerSensorGPIO());

	fd = open(buf, O_WRONLY);
	if( bBuzz == 1)
	{

        printf("calling sound 1\n");
		write(fd,"1",1); //switch on
	}
	else
	{
	    printf("calling sound 0\n");
		write(fd,"0",1); //switch off
	}
	close(fd);
}

void vTriggerMotionAction(bool bAction)
{
	if(bAction)
	{
		vWritetoLightSensor(true);
		//send sms
		if(g_configdata.bGetSMS())
		{
		    system("python /home/pi/send_sms.py");
		}
	}
	else
	{
		vWritetoLightSensor(false);
	}
}

void vHandleMessage(char* buffer)
{

	int msg_type = buffer[0];
	int data = buffer[1];
    int num_tries=1;
	switch(msg_type)
	{
        case TIMER_MSG:
		printf("\nTimeout with state - %d",stStateDef.current_state);
        if(stStateDef.current_state == MOTION_DETECTED)
        {
            vTriggerMotionAction(false);

		m_pStreamer->vSwitchVideo(NETOFF);
		if(g_configdata.iGetBuzzerState()!=BUZZER_STATE_MOVE)
		{
			for(int i=0; i<g_numofobjsensor; i++)
			{
			    //create object sensor threads
			    pthread_create (&objectsensor[i], NULL, &cObjectSensor::vCreateGPIO, m_pObjectSensor[i]);
			    //pthread_create (&objectsensor[i], NULL, &cObjectSensor::vCreateGPIO, m_pObjectSensor[i]);
			    //m_pObjectSensor[i]->vReadGPIO();
			}

		}
            //vCreateTimer();
            /*for(int i=0; i<g_numofobjsensor; i++)
            {
                m_pObjectSensor[i]->vReadGPIO();
            }*/

            stStateDef.current_state = MOTION_NOT_DETECTED;

        }
        break;

		/* message received from object motion sensor */
		case OBJECTSENSOR_MSG:
			printf("\nChange in state - %d, current state - %d",data, stStateDef.current_state);

			if(data == MOTION_DETECTED)
			{
				if(g_configdata.iGetBuzzerState() == BUZZER_STATE_MOVE)
				{
					vWritetoBuzzerSensor(true);
				}
	/*			for(int i=0; i<g_numofobjsensor; i++)
                {
                    //create object sensor threads
                    m_pObjectSensor[i]->vCloseRead();
                    //pthread_create (&objectsensor[i], NULL, &cObjectSensor::vCreateGPIO, m_pObjectSensor[i]);
                    //m_pObjectSensor[i]->vReadGPIO();
                }*/


			}
			else if(data == MOTION_NOT_DETECTED)
			{
				vWritetoBuzzerSensor(false);
			if(g_configdata.iGetBuzzerState()!=BUZZER_STATE_MOVE)
			{

				for(int i=0; i<g_numofobjsensor; i++)
				{
				    //create object sensor threads
				    m_pObjectSensor[i]->vCloseRead();
				}
			}
			}

			//when motion is detected
			if((data == MOTION_DETECTED) && stStateDef.current_state!=MOTION_DETECTED)
			{
				vCreateTimer();
				if(g_configdata.iGetBuzzerState()!=BUZZER_STATE_MOVE)
				vWritetoBuzzerSensor(true);
				string s="motion:Motion is detected at " + currenttime();
				distributemessage(s);
				vTriggerMotionAction(true);
				m_pStreamer->vSwitchVideo(NETON);
				/*for(int i=0; i<g_numofobjsensor; i++)
				{
					//create object sensor threads
					m_pObjectSensor[i]->vCloseRead();
				}*/
				stStateDef.current_state = MOTION_DETECTED;

			}
		break;

		case GSTREAMER_PLAY_MSG:
		break;

		case GSTREAMER_STOP_MSG:
		break;

		case GSTREAMER_NETWORKERROR_MSG:
			system("sudo reboot");
		break;

		case GSTREAMER_HDERROR_MSG:
			printf("\nHarddisk error. Recording stopped ");
		break;

		case WATCHDOG_MSG:
            /*if(g_configdata.iNetworkAddress() == 0 && g_configdata.bGetWatchdog()) //net not available, so post watchdog message
            {
                system("sync");
                system("sudo reboot");

            }*/
		break;

		default:
		break;
	};
}

static void* vProcessMessage(void *data)
{
        char buffer[MAX_QUEUE_SIZE + 1];
        int must_stop = 0;

        //process the message queue
         do
         {

			ssize_t bytes_read;

			/* receive the message */
			bytes_read = cMessageQueue::iReceiveMessage(buffer);

			CHECK(bytes_read >= 0);

			buffer[bytes_read] = '\0';
			if (! strncmp(buffer, MSG_STOP, strlen(MSG_STOP)))
			{
				must_stop = 1;
			}
			else
			{
				//process the message here
				vHandleMessage(buffer);

			}
    } while (!must_stop);

}
void distributemessage(string str)
{

    for(int i =0; i<num_clients; i++)
    {
        write(socks[i],str.c_str(),str.length());
    }
}

void vSendInit(int newsockfd)
{

        //write initial messages
        string sms = "sms:" + IntToString(g_configdata.bGetSMS()) + ":";
        string timeout = "timeout:" + IntToString(g_configdata.iGetTimeout() )+ ":";
        string cams = "cams:" + IntToString(g_configdata.iGetcams() )+ ":";
        string port1 = "port1:" + IntToString(g_configdata.iGetPort() )+ ":";
        string port2 = "port2:" + IntToString(g_configdata.iGetport2() )+ ":";
        string port3 = "port3:" + IntToString(g_configdata.iGetport3() )+ ":";
        string port4 = "port4:" + IntToString(g_configdata.iGetport3() )+ ":";
        string light = "light:" + IntToString(g_configdata.iGetLightState() )+ ":";
        string sound = "sound:" + IntToString(g_configdata.iGetBuzzerState() )+ ":";



        write(newsockfd,version_num.c_str(),version_num.length());
        write(newsockfd,init_time.c_str(),init_time.length());

        write(newsockfd,sms.c_str(),sms.length());
        write(newsockfd,timeout.c_str(),timeout.length());
        write(newsockfd,cams.c_str(),cams.length());
        write(newsockfd,port1.c_str(),port1.length());
        write(newsockfd,port2.c_str(),port2.length());
        write(newsockfd,port3.c_str(),port3.length());
        write(newsockfd,port4.c_str(),port4.length());
        write(newsockfd,light.c_str(),light.length());

        write(newsockfd,sound.c_str(),sound.length());




}
static void* doprocessing(void *data)
{
    int *sock = (int*)data;
    int newsockfd = *sock;
     char buffer[256];
            //write initial messages
        string sms = "sms:" + IntToString(g_configdata.bGetSMS()) + ":";
        string timeout = "timeout:" + IntToString(g_configdata.iGetTimeout() )+ ":";
        string cams = "cams:" + IntToString(g_configdata.iGetcams() )+ ":";
        string port1 = "port1:" + IntToString(g_configdata.iGetPort() )+ ":";
        string port2 = "port2:" + IntToString(g_configdata.iGetport2() )+ ":";
        string port3 = "port3:" + IntToString(g_configdata.iGetport3() )+ ":";
        string port4 = "port4:" + IntToString(g_configdata.iGetport3() )+ ":";
        string light = "light:" + IntToString(g_configdata.iGetLightState() )+ ":";
        string sound = "sound:" + IntToString(g_configdata.iGetBuzzerState() )+ ":";

     int n;

            if(num_clients >  100)
        {
            num_clients = 0;
        }

        //socks[num_clients] = newsockfd;
        //num_clients++;

        vSendInit(newsockfd);
   /* If connection is established then start communicating */
        while(1)
        {
            bzero(buffer,256);
            n = read( newsockfd,buffer,255 );
            if(n<0 || buffer==NULL)
		{
			printf("\n I am here in error");
                	break;
		}
            char * pch;


            pch = strtok (buffer,":");
            while (pch != NULL)
              {

                if(0==strcmp(pch,"sms"))
                {
                    pch = strtok (NULL, ":");
                    g_configdata.vSetSMSState(atoi(pch));
			printf("sms value is %d",atoi(pch));
                    sms = "sms:" + IntToString(g_configdata.bGetSMS()) + ":";
                    //distributemessage(sms);
                }
                else if(0==strcmp(pch,"light"))
                {
                    pch = strtok (NULL, ":");
                    g_configdata.vSetLightState(atoi(pch));
                    light = "light:" + IntToString(g_configdata.iGetLightState() )+ ":";
                    //distributemessage(light);
                }
                else if(0==strcmp(pch,"sound"))
                {
                    pch = strtok (NULL, ":");
                    g_configdata.vSetBuzzerState(atoi(pch));
                    sound = "sound:" + IntToString(g_configdata.iGetBuzzerState() )+ ":";
                    //distributemessage(sound);
                }
                else if(0==strcmp(pch,"timeout"))
                {
                    pch = strtok (NULL, ":");
                    g_configdata.vSetTimeout(atoi(pch));
                    timeout = "timeout:" + IntToString(g_configdata.iGetTimeout() )+ ":";
                    //distributemessage(timeout);
                }
                else if(0==strcmp(pch,"savereboot"))
                {
                    g_configdata.vWriteData_App();

                    sleep(2);
                    system("reboot");
                }
                else if(0==strcmp(pch,"nlight"))
                {
                    pch = strtok (NULL, ":");
		    int value=0;
		    value = atoi(pch);
                    printf("%d", atoi(pch));
		    printf("%d",value);

                    if(value == 1)
                    vJustWritetoLightSensor(true);
                    else
                    vJustWritetoLightSensor(false);
                }
                else if(0==strcmp(pch,"nsound"))
                {
                    pch = strtok (NULL, ":");
                    int val2 = atoi(pch);
                    if(val2 == 1)
                    vJustWritetoBuzzerSensor(true);
                    else
                    vJustWritetoBuzzerSensor(false);
                }
                else if(0==strcmp(pch,"clear"))
                {
                    string clr = "rm -r " + string(g_configdata.pGetFileLocation()) + "*";

                    printf("%s",clr.c_str());
                    system(clr.c_str());
                    //sleep(2);
                }
                else if (0==strcmp(pch,"resend"))
                {
                    pch = strtok (NULL, ":");
                    vSendInit(newsockfd);

                }
                else if(0==strcmp(pch,"clearnet"))
                {
                    string clr = "rm -r " + string(g_configdata.pGetNetworkFileLocation()) + "*";

                    printf("%s",clr.c_str());
                    system(clr.c_str());
                    //sleep(2);
                }
                else if(0==strcmp(pch,"reboot"))
                {
                    sleep(2);
                    system("reboot");
                }
                else if(0==strcmp(pch,"exit"))
                {
			printf("\nI am in exit");
		    close(newsockfd);
                    return NULL;
                }

                pch = strtok (NULL, ":");

              }


        }

}



static void* vSocketConnection(void* data)
{

    int sockfd, newsockfd, portno ;
    socklen_t clilen;
    pthread_t pid;
   char buffer[256];
   struct sockaddr_in serv_addr, cli_addr;
   int  n;

   /* First call to socket() function */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);

   if (sockfd < 0) {
      printf("ERROR opening socket");
      exit(1);
   }

   /* Initialize socket structure */
   bzero((char *) &serv_addr, sizeof(serv_addr));
   portno = g_configdata.iGetcommport();

   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(portno);

   /* Now bind the host address using bind() call.*/
   if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      printf("ERROR on binding");
      exit(1);
   }

   /* Now start listening for the clients, here process will
      * go in sleep mode and will wait for the incoming connection
   */

   listen(sockfd,5);
   clilen = sizeof(cli_addr);

   /* Accept actual connection from the client */
   while(1)
   {
       newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

       if (newsockfd < 0) {
          printf("ERROR on accept");
         exit(1);

       }
	printf("\nNew connection");
        pthread_create(&pid,NULL,doprocessing,&newsockfd);

   }

   return 0;
}



/// main(); start of execution
int main()
{

	g_configdata.vReadConfigData();

	init_time="init_time:"+currenttime()+":";


	//creating components
	vCreateComponents();

    vWritetoLightSensor(true);
	pthread_t gstthread; /* thread for gstreamer */
	//pthread_t objectsensor[NUMBER_OBJECT_SENSOR];  /* thread for object sensor */
	pthread_t workerqueue; /* thread for processing worker messages */


	/* create message queue*/
	static mqd_t* mqWorker = cMessageQueue::vCreateMessageQueue();

	if(mqWorker == NULL)
	{
		traceoutput(FATAL, "\nMessage Queue not created");

	}
    /* create worker thread */
    pthread_create (&workerqueue, NULL, &vProcessMessage, (void *) &mqWorker);


    pthread_create (&gstthread, NULL, &cStreamer::vStreamVideo, (void *) m_pStreamer);


    pthread_create (&sockcomm, NULL, &vSocketConnection, (void *) &mqWorker);








//vCreateTimer();
    for(int i=0; i<g_numofobjsensor; i++)
    {
	//create object sensor threads
		pthread_create (&objectsensor[i], NULL, &cObjectSensor::vCreateGPIO, m_pObjectSensor[i]);
		//m_pObjectSensor[i]->vReadGPIO();
	}

    vCreateTimer();

    while(1)
	{
		sleep(60);
	}

	/* Main block now waits for both threads to terminate, before it exits
       If main block exits, both threads exit, even if the threads have not
       finished their work */
    pthread_join(gstthread, NULL);
    /* exit */
    exit(0);
	return 0;

}

