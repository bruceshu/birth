


ifndef SUBDIR

IFLAGS     := -I. -I$(SRC_LINK)/
CPPFLAGS   := $(IFLAGS) $(CPPFLAGS)
CFLAGS     += $(ECFLAGS)
CCFLAGS     := $(CPPFLAGS) $(CFLAGS)



ALLFFLIBS = avcodec avdevice avfilter avformat avresample avutil postproc swscale swresample

define COMPILE
       $(call $(1)DEP,$(1))
       $($(1)) $($(1)FLAGS) $($(1)_DEPFLAGS) $($(1)_C) $($(1)_O) $(patsubst $(SRC_PATH)/%,$(SRC_LINK)/%,$<)
endef

COMPILE_C = $(call COMPILE,CC)

%.o: %.c
	$(COMPILE_C)
	
endif

include $(SRC_PATH)/ffbuild/arch.mak

OBJS      := $(sort $(OBJS:%=$(SUBDIR)%))
