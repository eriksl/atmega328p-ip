.c.o:
			gcc -c -Wall -O3 $< -o $@

all:		espif

espif:		espif.o
			gcc -O3 -s $< -o $@

install:
			install -o root -g adm -m 751 espif /usr/local/bin

clean:
			rm -f *.o espif
