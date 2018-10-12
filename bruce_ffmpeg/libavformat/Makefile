NAME = avformat
DESC = ffmpeg container format library

HEADERS = avformat.h \
		  avio.h \
		  url.h \
		  internal.h \
		  
OBJS = avio.o \
	   os_support.o \
	   protocols.o \
	   url.o \


OBJS-$(CONFIG_NETWORK) += network.o

# protocols I/O
OBJS-$(CONFIG_HTTP_PROTOCOL) += http.o httpauth.o


