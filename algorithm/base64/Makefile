

CC=gcc

CFLAGS += -Wall -O3
LDFLAGS += 

source = $(wildcard *.c)
obj += $(subst .c,.o,$(source))
#$(warning $(obj))

all:out

$(obj):%.o:%.c
	$(CC) $(CFLAGS) -c $< 
	
out:$(obj)
	$(CC) -o $@ $^
	
clean:
	rm -rf out $(obj)
	
.PHONY:all clean
