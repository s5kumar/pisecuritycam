#include <gst/gst.h>
#include "common_header.h"
#include <semaphore.h>
#include <signal.h>

#ifndef CSTREAMER_H_
#define CSTREAMER_H_
class cStreamer
{
	public:

	    static gboolean net_bus_call(GstBus *bus, GstMessage *msg, void *user_data);
		static void* vStreamVideo(void* data);
		void vStartNetworkVideo();
        void vInitVideo();
		void vStopNetworkVideo();
		void vSwitchVideo(int mode);
		static cStreamer* pGetInstance(const char* file_location = NULL, const char* ip_address=NULL, int port=0, int mode=2, const char* net_file_location=NULL, int maxfile=5, int framerate=5, int bitrate=1000000, int width=640, int height=480);

		void vSetIpAddress(char* ip_address)
		{
		    strcpy(m_ip_address, ip_address);

		}

	private:
		static cStreamer* m_gstInstance; //singleton instance

		cStreamer(const char* file_location, const char* ip_address,int port, int mode, const char* net_file_location, int maxfile,int framerate, int bitrate, int width, int height);	//constructor

		static GstElement* net_pipeline;

		static GstElement* local_pipeline;

		~cStreamer();	//destructor


	private:
        int m_currentstate;

		int saving_file_count;


		char* m_file_loc;

		char* m_net_file_loc;

		char* m_ip_address;

		int m_mode;

		int m_port;

        int m_maxfiles;

        int m_framerate;

        int m_bitrate;

        int m_width;

        int m_height;

        char m_filepath[64];

		GstPad *net_tee_q1_pad, *net_tee_q2_pad;

		GstPad *net_q1_pad, *net_q2_pad;
};
#endif //CSTREAMER_H_
