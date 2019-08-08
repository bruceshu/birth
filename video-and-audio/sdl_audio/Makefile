

CC=gcc
CFLAGS += -Wall

LIBS += -L./lib -lSDL2 -lSDL2main -lm -ldl -lpthread -lrt
INCLUDE=-I./include/SDL2

SOURCES := audio_out.c
OBJ := $(subst .c,.o,$(SOURCES))

all:out

%.o:%.c
	$(CC) $(CFLAGS) -c $< $(INCLUDE)

out:$(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(INCLUDE) $(LIBS)

clean:
	rm -rf out $(OBJ)

.PHONY: all clean
