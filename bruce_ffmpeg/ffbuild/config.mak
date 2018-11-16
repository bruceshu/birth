
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
LD=gcc
LD_O=-o $@

LIB_INSTALL_EXTRA_CMD=$$(RANLIB) "$(LIBDIR)/$(LIBNAME)"
INSTALL=install
STRIP=strip
STRIPTYPE=direct

# compile static option
CFLAGS= -std=c11 -fomit-frame-pointer -pthread -I/usr/include/ -g -Wdeclaration-after-statement -Wall -Wdisabled-optimization -Wpointer-arith -Wredundant-decls -Wwrite-strings -Wtype-limits -Wundef -Wmissing-prototypes -Wno-pointer-to-int-cast -Wstrict-prototypes -Wempty-body -Wno-parentheses -Wno-switch -Wno-format-zero-length -Wno-pointer-sign -O3 -fno-math-errno -fno-signed-zeros -fno-tree-vectorize -Werror=format-security -Werror=implicit-function-declaration -Werror=missing-prototypes -Werror=return-type -Werror=vla -Wformat -Wno-maybe-uninitialized

LD_LIB=-l%
LDFLAGS=  -Wl,--as-needed -Wl,-z,noexecstack -Wl,--warn-common -Wl,-rpath-link=libavformat:libavcodec:libavutil
LDEXEFLAGS=


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