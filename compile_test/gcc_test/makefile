

# flags
CC = gcc
CFLAGS = -Wall
LFLAGS =

# args 
RELEASE = 0
BITS = 

ifeq ($(RELEASE),0)
	# debug
	CFLAGS += -g
#	$(warning $(CFLAGS))
else
	# release
	CFLAGS += -static -O3 -DNDEBUG
	LFLAGS += -static
#	$(warning $(CFLAGS))
endif

ifeq ($(BITS),32)
	CFLAGS += -m32
	LFLAGS += -m32
else
	ifeq ($(BITS),64)
		CFLAGS += -m64
		LFLAGS += -m64
	else
	endif
endif

# file
TARGETS = gcc_make

all:$(TARGETS)

gcc_make:gcc_make.o
	$(CC) $(LFLAGS) -o $@ $^

gcc_make.o:gcc_make.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf gcc_make.o $(TARGETS) $(addsuffix .exe,$(TARGETS))

.PHONY: all clean
