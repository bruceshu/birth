PROJECT := udp_server udp_client
LIB = -lpthread

%.o:%.c
	gcc -c $<

all:$(PROJECT)

udp_server:udp_server.o
	gcc -o $@ $< $(LIB)

udp_client:udp_client.o
	gcc -o $@ $< $(LIB)

clean:
	-rm -rf *.o $(PROJECT)

.PHONY: PROJECT all clean
