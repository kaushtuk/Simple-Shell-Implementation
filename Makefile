osh : simple-shell.c
	gcc simple-shell.c -o osh.bin

clean : osh.bin
	rm -rf osh.bin

all : clean osh
