




PATH_LIBNAME = $(foreach NAME,$(1),lib$(NAME)/$($(2)LIBNAME))
DEP_LIBS := $(foreach lib,$(FFLIBS),$(call PATH_LIBNAME,$(lib),$(CONFIG_SHARED:yes=S)))
STATIC_DEP_LIBS := $(foreach lib,$(FFLIBS),$(call PATH_LIBNAME,$(lib)))

LDLIBS       = $(FFLIBS:%=%$(BUILDSUF))
FFEXTRALIBS := $(LDLIBS:%=$(LD_LIB)) $(foreach lib,EXTRALIBS-$(NAME) $(FFLIBS:%=EXTRALIBS-%),$($(lib))) $(EXTRALIBS)

OBJS      += $(OBJS-yes)
OBJS      := $(sort $(OBJS:%=$(SUBDIR)%))
#SLIBOBJS  += $(SLIBOBJS-yes)
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