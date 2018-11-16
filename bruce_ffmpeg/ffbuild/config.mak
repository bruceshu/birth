
ifndef FFMPEG_CONFIG_MAK
FFMPEG_CONFIG_MAK=1
FFMPEG_CONFIGURATION=
prefix=/usr/local
LIBDIR=$(DESTDIR)${prefix}/lib
INCDIR=$(DESTDIR)${prefix}/include
PKGCONFIGDIR=$(DESTDIR)${prefix}/lib/pkgconfig
SRC_PATH=.
SRC_LINK=.

ifndef MAIN_MAKEFILE
SRC_PATH:=$(SRC_PATH:.%=..%)
endif

# lib name
BUILDSUF=
LIBPREF=lib
FULLNAME=$(NAME)$(BUILDSUF)
LIBSUF=.a
LIBNAME=$(LIBPREF)$(FULLNAME)$(LIBSUF)

# prog name
PROGSSUF=
EXESUF=

# compile tool chain
#ARCH=x86
CC=gcc
CC_C=-c
CC_E=-E -o $@
CC_O=-o $@

CXX=g++
AR=ar
ARFLAGS=rcD
AR_O=$@
RANLIB=ranlib -D
LIB_INSTALL_EXTRA_CMD=$$(RANLIB) "$(LIBDIR)/$(LIBNAME)"
INSTALL=install
STRIP=strip
STRIPTYPE=direct

# compile static option
LD_LIB=-l%



# config prog
CONFIG_FFPROBE=yes

# config lib
CONFIG_AVFORMAT=yes

# config format
CONFIG_HTTP_PROTOCOL=yes
CONFIG_TCP_PROTOCOL=yes
CONFIG_MPEGTS_DEMUXER=yes

# config codec
CONFIG_AVCODEC=yes

# config compile
CONFIG_STATIC=yes

endif