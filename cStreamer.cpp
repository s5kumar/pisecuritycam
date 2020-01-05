#include <gst/gst.h>
#include "cStreamer.h"
#include "common_header.h"
#include "cMessageQueue.h"
#include <stdio.h>
#include <string.h>

cStreamer* cStreamer::m_gstInstance=NULL;
GMainLoop* g_loop = NULL;
GstElement* cStreamer::net_pipeline=NULL;
GstElement* net_q1 = NULL;
GstElement* net_tcpsink = NULL;






gboolean cStreamer::net_bus_call(GstBus *bus, GstMessage *msg, void *user_data)
{
    const gchar *message_name;
	switch (GST_MESSAGE_TYPE(msg)) {
		case GST_MESSAGE_EOS:
			g_main_loop_quit(g_loop);
		break;

		case GST_MESSAGE_ERROR:
			GError *err;
			gst_message_parse_error(msg, &err, NULL);
			//report error
			printf ("ERROR: %s", err->message);
			g_error_free(err);

			//cStreamer::pGetInstance()->vStopNetworkVideo();
			//g_main_loop_quit(g_loop);
			//system("sync");
			//system("sudo reboot");

		break;
		case GST_MESSAGE_STATE_CHANGED:
			GstState old, news, pending;
			gst_message_parse_state_changed (msg, &old, &news, &pending);
		break;

		default:
			message_name = gst_structure_get_name(gst_message_get_structure(msg));
			if(!strcmp(message_name, "GstMultiFileSink"))
            {

                if(cStreamer::pGetInstance()->m_net_file_loc != NULL)
                {
                    char copy_command[100];

                    sprintf(copy_command,"sudo mv  %s%s%d.h264 %sR_%s",cStreamer::pGetInstance()->m_file_loc,"Camera_",cStreamer::pGetInstance()->saving_file_count++,cStreamer::pGetInstance()->m_file_loc,cStreamer::pGetInstance()->m_filepath);
                    printf(copy_command);
                    system(copy_command);

		    sprintf(copy_command,"sudo mv %sR_* %s", cStreamer::pGetInstance()->m_file_loc, cStreamer::pGetInstance()->m_net_file_loc);
		    printf(copy_command);
		    system(copy_command);

                    char c_time_string[20];
                    time_t current_time;
                    struct tm* timeinfo;


                    time(&current_time);
                    timeinfo=localtime(&current_time);
                    strftime(c_time_string, 20, "%d%b%g_%H%M", timeinfo);
                    snprintf(cStreamer::pGetInstance()->m_filepath, 64, "%s.h264",c_time_string);

                }
            }
 		break;
	}
 return true;
}

void* cStreamer::vStreamVideo(void* data)
{
 //setenv("GST_DEBUG", "*:4", 1);
	gst_init (NULL, NULL);
	cStreamer* ptr = (cStreamer*) data;
	g_loop = g_main_loop_new(NULL, FALSE);
    //start recording local video here
    if(cStreamer::pGetInstance() == NULL)
    {
        traceoutput(INFO,"\nError in getInstance");
    }
    else
    {
        cStreamer::pGetInstance()->vInitVideo();
    }
    g_main_loop_run(g_loop);

}

/* Function call to start the Network video */
void cStreamer::vInitVideo()
{

    GstElement *net_filesink;
    GstElement *filter;
    GstElement *net_rpisrc,  *net_h264parse1, *net_h264parse2,*net_mux, *net_tee,  *net_q2,  *net_xenc;
    GstElement *net_flvmux1, *net_flvmux2;
    GstBus *net_bus;
	GstCaps *net_caps;

    char c_time_string[20];
    time_t current_time;
    struct tm* timeinfo;


    time(&current_time);
    timeinfo=localtime(&current_time);
//    strftime(c_time_string, 20, "%H%M_%d%b%g", timeinfo);
strftime(c_time_string, 20, "%d%b%g_%H%M", timeinfo);
  snprintf(m_filepath, 64, "%s.h264",c_time_string);

	/* Obtain current time as seconds elapsed since the Epoch. */

	net_pipeline = gst_pipeline_new ("video_pipeline");

	/* create the bus for the pipeline*/
	net_bus  = gst_pipeline_get_bus(GST_PIPELINE(net_pipeline));

	/* add the bus handler method */
	gst_bus_add_watch(net_bus, cStreamer::net_bus_call, g_loop);

	gst_object_unref(net_bus);

    //rpicamsrc-> filter -> h264parse -> flvmux -> tee -> q1 -> tcpserversink -> q2 -> multifilesink

    //rpicamsrc
	net_rpisrc = gst_element_factory_make ("rpicamsrc", "src");
	g_object_set(G_OBJECT(net_rpisrc), "bitrate",m_bitrate, NULL);
	g_object_set(G_OBJECT(net_rpisrc), "annotation-mode", 0x0000000d, NULL);
	g_object_set(G_OBJECT(net_rpisrc), "annotation-text", "Camera 1 : ", NULL);

	//filter
    filter = gst_element_factory_make ("capsfilter","filter");
    net_caps = gst_caps_new_simple("video/x-h264",
		"width", G_TYPE_INT, m_width,
		"height", G_TYPE_INT, m_height,
		"framerate", GST_TYPE_FRACTION,m_framerate,1,
		NULL);

	g_object_set(G_OBJECT(filter), "caps", net_caps, NULL);

	//h264parse
	net_h264parse1 = gst_element_factory_make ("h264parse", "net_h264parse1");
	//net_h264parse2 = gst_element_factory_make ("h264parse", "net_h264parse2");


    //flvmux
    net_flvmux1= gst_element_factory_make ("flvmux", "net_flvmux1");
    //net_flvmux2= gst_element_factory_make ("flvmux", "net_flvmux2");
    g_object_set(G_OBJECT(net_flvmux1), "streamable", true, NULL);

    //tee
    net_tee = gst_element_factory_make ("tee", "videotee");

	//q1
	net_q1 = gst_element_factory_make ("queue", "qone");

	//tcpserversink
    net_tcpsink = gst_element_factory_make ("tcpserversink", "tcpserversink");

	g_object_set (G_OBJECT (net_tcpsink), "host", m_ip_address, NULL);
	g_object_set (G_OBJECT (net_tcpsink), "port", m_port, NULL);

	//q2
	net_q2 = gst_element_factory_make ("queue", "qtwo");

    //multifilesink
    net_filesink = gst_element_factory_make ("multifilesink", "filesink");
    char file_path[64];
    snprintf(file_path, 64, "%s%s%s.h264",m_file_loc,"Camera_","%0d");

    g_object_set (G_OBJECT (net_filesink), "location", file_path, NULL);
    g_object_set (G_OBJECT (net_filesink), "async", true , NULL);


    g_object_set (G_OBJECT (net_filesink), "index", saving_file_count, NULL);
    g_object_set (G_OBJECT (net_filesink), "max-file-size", gint64(31457280), NULL);
    //g_object_set (G_OBJECT (net_filesink), "max-file-size", gint64(1048576), NULL);
    g_object_set (G_OBJECT (net_filesink), "next-file",4, NULL);

    //g_object_set (G_OBJECT (net_filesink), "max-files", m_maxfiles, NULL);
    g_object_set (G_OBJECT (net_filesink), "post-messages", true, NULL);


	//checks

    if(net_pipeline==NULL || net_rpisrc==NULL || net_h264parse1==NULL || net_flvmux1==NULL || net_tee==NULL || net_q1 ==NULL || net_q2 == NULL || net_tcpsink == NULL || net_filesink == NULL)
	{
		return;
	}

	//gst_bin_add_many(GST_BIN(net_pipeline), net_rpisrc, filter, net_tee, net_q1,net_h264parse1, net_flvmux1,net_tcpsink,net_q2,net_h264parse2, net_flvmux2,net_filesink,  NULL);
    if(m_mode != PLAY_LOCAL_MODE)
    gst_bin_add_many(GST_BIN(net_pipeline), net_rpisrc, filter, net_h264parse1, net_flvmux1, net_tee, net_q1,net_tcpsink,net_q2,net_filesink,  NULL);
    else
    gst_bin_add_many(GST_BIN(net_pipeline), net_rpisrc, filter, net_h264parse1, net_flvmux1, net_tee, net_q2,net_filesink,  NULL);
	/* Link the camera source and csp filter using capabilities
	* specified */

	if(!gst_element_link_many(net_rpisrc,  filter, net_h264parse1, net_flvmux1, net_tee,NULL))
	{
	gst_object_unref (net_pipeline);
	return;
	}

//	gst_object_unref(net_caps);

	/* Link the first sink */
	//if(!gst_element_link_many(net_q1, net_h264parse1, net_flvmux1, net_tcpsink, NULL))


	/* Link the second sink */
	//if(!gst_element_link_many(net_q2, net_h264parse2, net_flvmux2, net_filesink, NULL))
	if(!gst_element_link_many(net_q2,  net_filesink, NULL))
	{
		gst_object_unref (net_pipeline);
		return;
	}
	GstPadTemplate *net_tee_src_pad_template;


	/* Manually link the Tee, which has "Request" pads */
	if ( !(net_tee_src_pad_template = gst_element_class_get_pad_template (GST_ELEMENT_GET_CLASS (net_tee), "src_%u"))) {
	gst_object_unref (net_pipeline);
	return;
	}

	net_tee_q2_pad = gst_element_request_pad (net_tee, net_tee_src_pad_template, NULL, NULL);
	//printf("Obtained request pad %s for q2 branch.\n", gst_pad_get_name (net_tee_q2_pad));
	net_q2_pad = gst_element_get_static_pad (net_q2, "sink");


	/* Link the tee to the queue 1 */
    if(m_mode != PLAY_LOCAL_MODE)
    {
        if(!gst_element_link_many(net_q1, net_tcpsink, NULL))
        {
            gst_object_unref (net_pipeline);
            return;
        }

        /* Obtaining request pads for the tee elements*/
        net_tee_q1_pad = gst_element_request_pad (net_tee, net_tee_src_pad_template, NULL, NULL);
        //printf ("Obtained request pad %s for q1 branch.\n", gst_pad_get_name (net_tee_q1_pad));
        net_q1_pad = gst_element_get_static_pad (net_q1, "sink");


        if (gst_pad_link (net_tee_q1_pad, net_q1_pad) != GST_PAD_LINK_OK ){

            gst_object_unref (net_pipeline);
            return;
        }
    }


	/* Link the tee to the queue 2 */
	if (gst_pad_link (net_tee_q2_pad, net_q2_pad) != GST_PAD_LINK_OK) {

        gst_object_unref (net_pipeline);
	return;
	}

    gst_element_set_state(net_pipeline,GST_STATE_PLAYING);

}


/* Function call to start the Network video */

void cStreamer::vStartNetworkVideo()
{
	if(m_mode == PLAY_LOCALANDNETWORK_MODE)
	{
		//gst_pad_link (net_tee_q1_pad, net_q1_pad);
        gst_element_link(net_q1,net_tcpsink);
	gst_element_set_state(net_tcpsink,GST_STATE_PLAYING);
	char send_data[2];
        send_data[0] = WATCHDOG_MSG;
        send_data[1]='\0';
        cMessageQueue::vPostMessage(send_data);
	}
}


/* Function call to stop the Network video */
void cStreamer::vStopNetworkVideo()
{
	if( m_mode == PLAY_LOCALANDNETWORK_MODE)
	{
        gst_element_unlink(net_q1, net_tcpsink);
	gst_element_set_state(net_tcpsink,GST_STATE_NULL);
        //gst_pad_unlink(net_tee_q1_pad, net_q1_pad);
 	}
}
void cStreamer::vSwitchVideo(int mode)
{
	switch(mode)
	{
		case NETON:
				vStartNetworkVideo();
		break;
		case NETOFF:
				vStopNetworkVideo();
		break;
		default:
		break;
	}
}

/***********************************************************************
 * Base Functions
 ***********************************************************************/
cStreamer* cStreamer::pGetInstance(const char* file_location, const char* ip_address, int port, int mode, const char* net_file_location, int maxfile, int framerate, int bitrate, int width, int height)
{
	if(m_gstInstance == NULL)
	{
		m_gstInstance = new cStreamer(file_location, ip_address, port,mode, net_file_location, maxfile, framerate, bitrate, width,height);
	}
	return m_gstInstance;
}

cStreamer::cStreamer(const char* file_location, const char* ip_address, int port, int mode, const char* net_file_location, int maxfile, int framerate, int bitrate, int width, int height)
{
	//do nothing
	m_currentstate = 0;
	saving_file_count = 0;
	m_maxfiles = maxfile;
	m_framerate = framerate;
	m_bitrate = bitrate;
	m_width = width;
	m_height = height;
	m_ip_address = new char[strlen(ip_address) + 1];
	strcpy(m_ip_address,ip_address);
	m_port = port;
	m_mode = mode;
	m_file_loc = new char[strlen(file_location)];
	if(net_file_location!=NULL)
	m_net_file_loc = new char[strlen(net_file_location)];
	//m_file_loc = (char*)malloc(strlen(file_location)+1);

	strcpy(m_file_loc,file_location);
	strcpy(m_net_file_loc, net_file_location);
	//	bModeSwitchInProgress = false;
}

cStreamer::~cStreamer()
{
	if(m_gstInstance)
	{
		delete m_gstInstance;
		m_gstInstance = NULL;
	}

}

