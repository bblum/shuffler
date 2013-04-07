all: shuffler

shuffler: shuffler.c bmplib.h bmplib-int.h bmplib.c
	gcc -Wall -Werror -O1 shuffler.c bmplib.c -o shuffler

clean:
	rm -f shuffler
