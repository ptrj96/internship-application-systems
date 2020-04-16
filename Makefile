CC = gcc
CFLAGS = -g -Wall

all: myping

myping.o: myping.c
	$(CC) $(CFLAGS) -c myping.c

myping: myping.o
	$(CC) $(CFLAGS) myping.o -o myping -lm

clean:
	rm -r myping *.o

