

CC=gcc

CFLAGS += -Wall -O3
LDFLAGS +=

src = $(wildcard *.c)
obj = $(subst .c,.o,$(src))
$(warning $(obj))

all:out

$(obj):%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
	
out:$(obj)
	$(CC) $(CFLAGS) -o $@ $^
	
clean:
	rm -rf out $(obj)
	
.PHONY: all clean
