CC = gcc
CFLAGS = -g -Wall

all: ping

ping.o: ping.c
	$(CC) $(CFLAGS) -c ping.c

ping: ping.o
	$(CC) $(CFLAGS) ping.o -o ping

clean:
	rm -r ping *.o

