project := http_download

lib += -lpthread
cflag += -std=c99

%.o:%.c
	gcc -c $< $(cflag)

all:$(project)

http_download:main.o http_download.o
	gcc -o %@ %< $(cflag) $(lib)

clean:
	rm -rf *.o $(project)

.PHONY: project all clean