CC = gcc
CFLAGS = -Wall -pthread

all : boating 

boating : boating.c
	$(CC) $(CFLAGS) -o boating boating.c

run : boating
	./boating 5 30

clean : 
	rm -f boating
	