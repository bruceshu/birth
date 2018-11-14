

#$(warning common_mak SUBDIR=$(SUBDIR))

ifndef SUBDIR

ifndef V
Q      = @
RM	   =rm -rf
ECHO   = printf "$(1)\t%s\n" $(2)
SILENT = RM

M      = @$(call ECHO,$(TAG),$@);

$(foreach VAR,$(SILENT),$(eval override $(VAR) = @$($(VAR))))
$(eval INSTALL = @$(call ECHO,INSTALL,$$(^:$(SRC_DIR)/%=%)); $(INSTALL))
#$(warning INSTALL=$(INSTALL))
endif

ALLFFLIBS = avcodec avformat

IFLAGS     := -I. -I$(SRC_LINK)/
CPPFLAGS   := $(IFLAGS) $(CPPFLAGS)
CFLAGS     += $(ECFLAGS)
CCFLAGS    := $(CPPFLAGS) $(CFLAGS)

define COMPILE
       $(call $(1)DEP,$(1))
       $($(1)) $($(1)FLAGS) $($(1)_DEPFLAGS) $($(1)_C) $($(1)_O) $(patsubst $(SRC_PATH)/%,$(SRC_LINK)/%,$<)
endef

COMPILE_C = $(call COMPILE,CC)

%.o: %.c
	$(COMPILE_C)
	
%.c %.h %.pc %.ver %.version: TAG = GEN
endif

#include $(SRC_PATH)/ffbuild/arch.mak


OBJS      += $(OBJS-yes)
#SLIBOBJS  += $(SLIBOBJS-yes)
FFLIBS    := $(FFLIBS-yes) $(FFLIBS)
#PATH_LIBNAME = $(foreach NAME,$(1),lib$(NAME)/$($(2)LIBNAME))
#DEP_LIBS := $(foreach lib,$(FFLIBS),$(call PATH_LIBNAME,$(lib),$(CONFIG_SHARED:yes=S)))
#LDLIBS       = $(FFLIBS:%=%$(BUILDSUF))

OBJS      := $(sort $(OBJS:%=$(SUBDIR)%))
#SLIBOBJS  := $(sort $(SLIBOBJS:%=$(SUBDIR)%))

#$(warning common_mak OBJS=$(OBJS))
#$(warning common_mak SLIBOBJS=$(SLIBOBJS))

#HOSTOBJS  := $(HOSTPROGS:%=$(SUBDIR)%.o)
HEADERS   += $(HEADERS-yes)

CLEANSUFFIXES     = *.d *.o *.pc *.ver *.version 
LIBSUFFIXES       = *.a *.so *.so.*

#define RULES
#clean::
#	$(RM) $(HOSTPROGS)
#endef

#$(eval $(RULES))

#-include $(wildcard $(OBJS:.o=.d) $(HOSTOBJS:.o=.d) $(TESTOBJS:.o=.d) $(HOBJS:.o=.d) $(SLIBOBJS:.o=.d)) $(OBJS:.o=$(DEFAULT_X86ASMD).d)