objects = main.o fun1.o fun2.o fun3.o

out : $(objects)
	gcc -o out $(objects)

.PHONY : clean
clean :
	-rm out *.o
