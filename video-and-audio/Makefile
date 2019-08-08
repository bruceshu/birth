

CC=gcc
CFLAGS += -Wall

LIBS += -L./lib -lSDL2 -lSDL2main -lm -ldl -lpthread -lrt
INCLUDE = -I./include/SDL2

video_path = sdl_video
audio_path = sdl_audio

video_src := $(video_path)/video_out.c
audio_src := $(audio_path)/audio_out.c

video_obj := $(subst .c,.o,$(video_src))
audio_obj := $(subst .c,.o,$(audio_src))
#$(warning $(video_obj))

objs += $(video_obj) $(audio_obj)

all: video_out audio_out

$(objs):%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDE)

video_out:$(video_obj)
	$(CC) $(CFLAGS) -o $@ $(video_obj) $(INCLUDE) $(LIBS)

audio_out:$(audio_obj)
	$(CC) $(CFLAGS) -o $@ $(audio_obj) $(INCLUDE) $(LIBS)


clean:
	rm -rf audio_out video_out $(video_obj) $(audio_obj)

.PHONY: all clean
