CC=gcc
CFLAGS=-pedantic -Wall -std=gnu99 -I/local/courses/csse2310/include
LFLAGS=-L/local/courses/csse2310/lib -lcsse2310a1

uqwordladder: uqWordLadder.o
	$(CC) $(LFLAGS) -o $@ $<

uqWordLadder.o: uqWordLadder.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f uqwordladder uqWordLadder.o
