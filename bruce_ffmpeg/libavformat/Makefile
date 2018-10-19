NAME = avformat
DESC = ffmpeg container format library

HEADERS = avformat.h \
		  avio.h \
		  version.h \
		  
OBJS = avformat.0 \
	   avio.o \
	   url.o \
	   dump.o \
	   os_support.o \
	   protocols.o \
	   utils.o \


OBJS-$(CONFIG_NETWORK) += network.o

# protocols I/O
OBJS-$(CONFIG_TCP_PROTOCOL)  			 += tcp.o
OBJS-$(CONFIG_HTTP_PROTOCOL) 		   	 += http.o httpauth.o

# muxers/demuxers
OBJS-$(CONFIG_AAC_DEMUXER)          	 += aacdec.o apetag.o img2.o rawdec.o
OBJS-$(CONFIG_AC3_DEMUXER)          	 += ac3dec.o rawdec.o
OBJS-$(CONFIG_AC3_MUXER)                 += rawenc.o
OBJS-$(CONFIG_MPEGTS_DEMUXER)            += mpegts.o
OBJS-$(CONFIG_MPEGTS_MUXER)              += mpegtsenc.o

$(warning $(NAME)_makefile $(OBJS))
