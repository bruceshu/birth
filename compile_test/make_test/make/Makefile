
objects = main.o str.o

out:$(objects)

CC = gcc

$(objects):%.o:%.c
	$(CC) -c $< -o $@

clean:
	rm out
