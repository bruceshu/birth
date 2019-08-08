

CC=gcc
CFLAGS += -Wall

video_src := sdl_video/video_out.c
audio_src := sdl_audio/audio_out.c

video_obj := $(subst .c,.o,$(video_src))
audio_obj := $(subst .c,.o,$(audio_src))

all:video_out audio_out

%.o:%.c
	$(CC) $(CFLAGS) -c $< $(INCLUDE)

video_out:$(video_obj)
	$(CC) $(CFLAGS) -o $@ $(video_obj) $(INCLUDE) $(LIBS)

audio_out:$(audio_obj)
	$(CC) $(CFLAGS) -o $@ $(audio_obj) $(INCLUDE) $(LIBS)


clean:
	rm -rf audio_out video_out $(video_obj) $(audio_obj)

.PHONY: all clean
