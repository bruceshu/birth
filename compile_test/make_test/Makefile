objects = main.o print.o

out : $(objects)
	cc -o out $(objects)

print.o : print.c
	gcc -c print.c -std=c99

.PHONY:clean
clean :
	-rm out $(objects)
