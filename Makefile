CC=g++
LIBS=`pkg-config gstreamer-1.0 --libs`  -lgstreamer-1.0
LIBS+=-lz -lrt -lm
CFLAGS=`pkg-config gstreamer-1.0 --cflags`
all:
	$(CC) $(LIBS) $(CFLAGS) cMessageQueue.cpp cObjectSensor.cpp common_header.cpp cStreamer.cpp cConfigdata.cpp main.cpp -o security.out


