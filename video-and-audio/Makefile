CC=gcc

LIBS += -L./lib -lSDL -lSDLmain -lm -ldl -lpthread -lrt

ANDROID_INCLUDE += -I$(ANDROID_COMPILE_ARM)/sysroot/usr/include -I$(ANDROID_COMPILE_ARM)/lib64/clang/3.8/include
SDL_INCLUDE=-I./include/sdl
INCLUDE += $(SDL_INCLUDE)

CFLAGS +=

audio_SRC := sdl_audio/audio_out.c
audio_OBJ := $(subst .c,.o,$(audio_SRC))

video_SRC := sdl_video/video_out.c
video_OBJ := $(subst .c,.o,$(video_SRC))

out += audio
 
all: $(out)

%.o:%.c
	$(CC) $(CFLAGS) -c $< $(INCLUDE)

audio:$(audio_OBJ)
	$(CC) $(CFLAGS) -o $@ $(audio_OBJ) $(INCLUDE) $(LIBS)
	
video:$(video_OBJ)
	$(CC) $(CFLAGS) -o $@ $(video_OBJ) $(INCLUDE) $(LIBS)

clean:
	rm -rf $(out) $(audio_OBJ) $(video_OBJ)

.PHONY: all clean
