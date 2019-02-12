CC=gcc

LIBS += -L. -lSDL -lSDLmain -lm -ldl -lpthread -lrt
ANDROID_INCLUDE += -I$(ANDROID_COMPILE_ARM)/sysroot/usr/include -I$(ANDROID_COMPILE_ARM)/lib64/clang/3.8/include
SDL_INCLUDE=-I./include/sdl
INCLUDE += $(SDL_INCLUDE)

SRC := audio_out.c
OBJ := $(subst .c,.o,$(SRC))

CFLAGS += -Wno-undef

all: out

%.o:%.c
	$(CC) $(CFLAGS) -c $< $(INCLUDE)

out:$(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(INCLUDE) $(LIB)

clean:
	rm -rf *.o out $(OBJ)

.PHONY: all clean
