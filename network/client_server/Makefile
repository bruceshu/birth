project := server client

lib += -lpthread

%.o:%.c
	gcc -c $<

all:$(project)

server:server.o
	gcc -o $@ $< $(lib)

client:client.o
	gcc -o $@ $< $(lib)	

clean:
	rm -rf *.o $(project)

.PHONY: project all clean