


SRC_PATH=.
ifndef MAIN_MAKEFILE
SRC_PATH:=$(SRC_PATH:.%=..%)
endif

CONFIG_AVFORMAT=yes



ALLFFLIBS = avcodec avdevice avfilter avformat avresample avutil postproc swscale swresample


