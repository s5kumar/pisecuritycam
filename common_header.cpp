#include "common_header.h"
#include <time.h>

#define ONLINE_DEBUG

void traceoutput(int log_level, const char * format, ...)
{

	va_list variable_arg_list;
	va_start(variable_arg_list,format);
	char* c_time_string;
    time_t current_time;

	//va_start(variable_arg_list, format);

	#ifdef ONLINE_DEBUG
	printf(format, variable_arg_list);


	#else
    current_time = time(NULL);


      /* Convert to local time format. */
    c_time_string = ctime(&current_time);

	FILE *f = fopen("/home/pi/file.txt", "a");
	if (f == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	fprintf(f,"%s : ",c_time_string);
	fprintf(f, format, variable_arg_list);
	fclose(f);
	//va_end(variable_arg_list);
	#endif

	if(log_level == FATAL)
	{
		//reset target here
	}
	va_end(variable_arg_list);
}




