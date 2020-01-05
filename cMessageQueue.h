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


#include "common_header.h"

#ifndef CMESSAGEQUEUE_H_
#define CMESSAGEQUEUE_H_

class cMessageQueue
{
	public:
		static mqd_t* vCreateMessageQueue();
		static int iReceiveMessage(char* buffer);
		static void vPostMessage(char* data);
		static void vCloseMessage(char* data);
		
		static mqd_t m_mqWorker;
};

#endif //CMESSAGEQUEUE_H_
