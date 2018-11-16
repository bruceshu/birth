
Q      = @
ECHO   = printf "$(1)\t%s\n" $(2)
BRIEF  = CC AR LD STRIP
SILENT = RM RANLIB

MSG    = $@
M      = @$(call ECHO,$(TAG),$@);

$(foreach VAR,$(BRIEF), $(eval override $(VAR) = @$$(call ECHO,$(VAR),$$(MSG)); $($(VAR))))
$(foreach VAR,$(SILENT),$(eval override $(VAR) = @$($(VAR))))
$(eval INSTALL = @$(call ECHO,INSTALL,$$(^:$(SRC_DIR)/%=%)); $(INSTALL))

#ALLFFLIBS = avcodec avformat

IFLAGS     := -I. -I$(SRC_LINK)/
CPPFLAGS   := $(IFLAGS) $(CPPFLAGS)
CFLAGS     += $(ECFLAGS)
CCFLAGS    := $(CPPFLAGS) $(CFLAGS)

define COMPILE
       $($(1)) $($(1)FLAGS) $($(1)_DEPFLAGS) $($(1)_C) $($(1)_O) $(patsubst $(SRC_PATH)/%,$(SRC_LINK)/%,$<)
endef

COMPILE_C = $(call COMPILE,CC)

%.o: %.c
	$(COMPILE_C)
	
%.c %.h %.pc %.ver %.version: TAG = GEN

FFLIBS    := $(FFLIBS-yes) $(FFLIBS)
