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

//#include "cMessageQueue.h"
//#include "cStreamer.h"
//#include "cUltrasonicSensor.h"
//#include "cObjectSensor.h"
//#include "cRotor.h"


#ifndef COMMON_H_
#define COMMON_H_

/* attributes for message queue */
#define QUEUE_NAME  "/worker_queue"
#define MAX_QUEUE_SIZE    1024
#define MAX_MSG_SIZE 128
#define MSG_STOP    "exit"
#define NUMBER_OBJECT_SENSOR 10 //to be moved to dynamic configuration


/* Message type definitions */

#define OBJECTSENSOR_MSG 1
#define GSTREAMER_PLAY_MSG 2
#define GSTREAMER_STOP_MSG 3
#define GSTREAMER_NETWORKERROR_MSG 4
#define GSTREAMER_HDERROR_MSG 5
#define WATCHDOG_MSG 6
#define TIMER_MSG 7

/* state machine events */
#define INIT 0
#define LOCAL_STREAM 1
#define NETWORK_STREAM 2

/* Object sensor events */
#define MOTION_NOT_DETECTED 0
#define MOTION_DETECTED 1


/* Play mode */
#define PLAY_NETWORK_MODE 0
#define PLAY_LOCAL_MODE 1
#define PLAY_LOCALANDNETWORK_MODE 2

/* Light state */
#define LIGHT_STATE_OFF 0
#define LIGHT_STATE_ON 1
#define LIGHT_STATE_NIGHT 2


/* Buzzer state */
#define BUZZER_STATE_OFF 0
#define BUZZER_STATE_ON 1
#define BUZZER_STATE_NIGHT 2
#define BUZZER_STATE_MOVE 3

/* switch video states */
#define NETON 0
#define NETOFF 1


#define CHECK(x) \
    do { \
        if (!(x)) { \
            fprintf(stderr, "%s:%d: ", __func__, __LINE__); \
            perror(#x); \
        } \
    } while (0) \

#define FATAL 0
#define DEBUG 1
#define INFO 2

#define TIMEOUT_SECS 1*60

class cObjectSensor;

//data to be sent to object sensor code
struct stObjectSensorData
{
	mqd_t* mqWorker;
	cObjectSensor* pObjectSensor;
};

//int g_ObjectSensorGPIO[NUMBER_OBJECT_SENSOR] = {22}; //to be moved to dynamic configuration

static bool g_bShutdownMessage = false;



//extern void traceoutput(int log_level, const char * format, ...);
#endif //COMMON_H_

extern void traceoutput(int log_level, const char * format, ...);

