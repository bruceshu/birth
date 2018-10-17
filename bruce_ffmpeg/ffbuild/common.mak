

$(warning common_mak SUBDIR=$(SUBDIR))

ifndef SUBDIR

ifndef V
Q      = @
ECHO   = printf "$(1)\t%s\n" $(2)
M      = @$(call ECHO,$(TAG),$@);
$(eval INSTALL = @$(call ECHO,INSTALL,$$(^:$(SRC_DIR)/%=%)); $(INSTALL))

$(warning INSTALL=$(INSTALL))
endif

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
	
%.c %.h %.pc %.ver %.version: TAG = GEN
endif

include $(SRC_PATH)/ffbuild/arch.mak


OBJS      += $(OBJS-yes)
SLIBOBJS  += $(SLIBOBJS-yes)
FFLIBS    := $($(NAME)_FFLIBS) $(FFLIBS-yes) $(FFLIBS)

#LDLIBS       = $(FFLIBS:%=%$(BUILDSUF))

OBJS      := $(sort $(OBJS:%=$(SUBDIR)%))
SLIBOBJS  := $(sort $(SLIBOBJS:%=$(SUBDIR)%))

$(warning common_mak OBJS=$(OBJS))
$(warning common_mak SLIBOBJS=$(SLIBOBJS))
#HOSTOBJS  := $(HOSTPROGS:%=$(SUBDIR)%.o)
#HEADERS   += $(HEADERS-yes)

CLEANSUFFIXES     = *.d *.gcda *.gcno *.h.c *.ho *.map *.o *.pc *.ptx *.ptx.c *.ver *.version *$(DEFAULT_X86ASMD).asm *~
LIBSUFFIXES       = *.a *.lib *.so *.so.* *.dylib *.dll *.def *.dll.a
define RULES
clean::
	$(RM) $(HOSTPROGS)
endef

$(eval $(RULES))

-include $(wildcard $(OBJS:.o=.d) $(HOSTOBJS:.o=.d) $(TESTOBJS:.o=.d) $(HOBJS:.o=.d) $(SLIBOBJS:.o=.d)) $(OBJS:.o=$(DEFAULT_X86ASMD).d)