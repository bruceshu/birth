MAIN_MAKEFILE=1
include ffbuild/config.mak

#在指定SRC_PATH路径中查找 *.c 和 *.h 文件
#vpath %.c $(SRC_PATH)
#vpath %.h $(SRC_PATH)

#目标编译的库名称
#FFLIBS-$(CONFIG_AVDEVICE) += avdevice
#FFLIBS-$(CONFIG_AVFILTER) += avfilter
FFLIBS-$(CONFIG_AVFORMAT) += avformat
#FFLIBS-$(CONFIG_AVCODEC) += avcodec
#FFLIBS-$(CONFIG_AVRESAMPLE) += avresample
#FFLIBS-$(CONFIG_POSTPROC) += postproc
#FFLIBS-$(CONFIG_SWRESAMPLE) += swresample
#FFLIBS-$(CONFIG_SWSCALE) += swscale
FFLIBS := avutil

$(warning main_makefile FFLIBS-yes=$(FFLIBS-yes))

#first so "all" becomes default target
all : all-yes

include $(SRC_PATH)/ffbuild/common.mak

$(warning main_makefile FFLIBS-yes=$(FFLIBS-yes))
$(warning main_makefile FFLIBS=$(FFLIBS))

SUBDIR_VARS := CLEANFILES FFLIBS OBJS   
			   
define RESET
$(1) :=
$(1)-yes :=
endef

define DOSUBDIR
$(foreach V,$(SUBDIR_VARS),$(eval $(call RESET,$(V))))
SUBDIR := $(1)/
include $(SRC_PATH)/$(1)/Makefile
-include $(SRC_PATH)/$(1)/$(ARCH)/Makefile
#-include $(SRC_PATH)/$(1)/$(INTRINSTICS)/Makefile
include $(SRC_PATH)/ffbuild/library.mak
endef

$(foreach P,$(FFLIBS),$(eval $(call DOSUBDIR,lib$(P))))

#include $(SRC_PATH)/fftools/Makefile



#all : all-yes

#include $(SRC_PATH)/ffbuild/common.mak


#install: install-libs install-headers

#install-libs: install-libs-yes

#uninstall: uninstall-data uninstall-headers uninstall-libs uninstall-pkgconfig

#uninstall-data:
#	$(RM) -r "$(DATADIR)"
	
clean::
	$(RM) $(OBJS-yes) $(OBJS)
	
.PHONY: clean
#.PHONY: all all-yes alltools check
#.PHONY: *clean install* uninstall*











