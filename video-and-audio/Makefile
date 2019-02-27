


#在指定SRC_PATH路径中查找 *.c文件
SRC_PATH=.
vpath %.c $(SRC_PATH)

audio_SRC := sdl_audio/audio_out.c
audio_OBJ := $(subst .c,.o,$(audio_SRC))

video_SRC := video_out.c
video_OBJ := $(subst .c,.o,$(video_SRC))

#编译工具
CC=gcc

#编译选项
CFLAGS += -Wall

#编译依赖的库文件
LIBS += -L./lib -lSDL -lSDLmain -lm -ldl -lpthread -lrt

#编译依赖的头文件
#ANDROID_INCLUDE += -I$(ANDROID_COMPILE_ARM)/sysroot/usr/include -I$(ANDROID_COMPILE_ARM)/lib64/clang/3.8/include
SDL_INCLUDE=-I./include/sdl
INCLUDE += $(SDL_INCLUDE)


#目标生成文件
out += audio

#脚本第一个目标，伪目标
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
