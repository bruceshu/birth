NAME = avutil
DESC = FFmpeg utility library


HEADERS =  avstring.h \
		   assert.h \
		   avutil.h \
		   common.h \
		   dict.h \
		   error.h \
		   frame.h \
		   mathematics.h \
		   mem.h \
		   opt.h \
		   pixfmt.h \
		   samplefmt.h \
		   

		  
HEADERS-$(CONFIG_LZO) += lzo.h


ARCH_HEADERS = 
			   
BUILT_HEADERS = 
				
OBJS = error.o \
	   log.o \
	   mem.o \
	   dict.o \
	   buffer.o \
	   avstring.o \
	   rational.o \
	   time.o \
	   mathematics.o \
	   frame.o \
	   
	   
	   
OBJS-$(CONFIG_CUDA) += hwcontext_cuda.o

OBJS += $(COMPAT_OBJS:%=../compat/%)

$(warning $(NAME)_makefile $(OBJS))



