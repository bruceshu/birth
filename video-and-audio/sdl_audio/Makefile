
OBJ := audio_out.o

CC=gcc
CFLAGS += -Wall
LIBS += -L./../lib -lSDL -lSDLmain -lm -ldl -lpthread -lrt
#ANDROID_INCLUDE += -I$(ANDROID_COMPILE_ARM)/sysroot/usr/include -I$(ANDROID_COMPILE_ARM)/lib64/clang/3.8/include
SDL_INCLUDE=-I./../include/sdl
INCLUDE += $(SDL_INCLUDE)

all:out

%.o:%.c
	$(CC) $(CFLAGS) -c $< $(INCLUDE)

out:$(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(INCLUDE) $(LIBS)

clean:
	rm -rf out $(OBJ)

.PHONY: all clean