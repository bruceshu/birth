out:$(subst .c,.o,$(wildcard *.c))
	gcc -o $@ $^

$.o:%.c
	gcc -o $@ $<

.PHONY : clean
clean :
	-rm out *.o
