NAME = avutil
DESC = FFmpeg utility library


HEADERS = avutil.h \
		  avstring.h \
		  
HEADERS-$(CONFIG_LZO) += lzo.h


ARCH_HEADERS = bswap.h \
			   intmath.h \
			   timer.h \
			   
BUILT_HEADERS = avconfig.h \
				ffversion.h
				
OBJS = error.o \
	   log.o \
	   mem.o \
	   
OBJS-$(CONFIG_CUDA) += hwcontext_cuda.o


OBJS += $(COMPAT_OBJS:%=../compat/%)



