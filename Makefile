all: shuffler

shuffler: shuffler.c bmplib.h bmplib-int.h bmplib.c mt19937int.h mt19937int.c
	gcc -Wall -Werror -O1 shuffler.c bmplib.c mt19937int.c -o shuffler

clean:
	rm -f shuffler
