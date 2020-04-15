CC = gcc
CFLAGS = -g -Wall

all: ping

ping.o: ping.C
	$(CC) $(CFLAGS) -c ping.C

ping: ping.o
	$(CC) $(CFLAGS) ping.o -o ping

clean:
	rm -r ping *.o

