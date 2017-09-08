default: bypass

bypass.o: bypass.c
	gcc -c bypass.c -o bypass.o

bypass: bypass.o
	gcc bypass.o -o bypass

clean:
	rm -f bypass.o
	rm -f bypass
