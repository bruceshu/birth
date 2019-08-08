PROJECT := server_user client_user
LIB = -lpthread

%.o:%.c
	gcc -c $<

all:$(PROJECT)

server_user:server_user.o
	gcc -o $@ $< $(LIB)

client_user:client_user.o
	gcc -o $@ $< $(LIB)

clean:
	-rm -rf *.o $(PROJECT)

.PHONY: PROJECT all clean
