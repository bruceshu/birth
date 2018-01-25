objects = main.o fun2.o fun3.o

out : $(objects)
	gcc -o out $(objects)

fun3.o : fun1.o

.PHONY : clean
clean :
	-rm out *.o
