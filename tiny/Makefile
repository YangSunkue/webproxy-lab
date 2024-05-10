CC = gcc
CFLAGS = -O2 -Wall -I .

# This flag includes the Pthreads library on a Linux box.
# Others systems will probably require something different.
LIB = -lpthread

all: tiny cgi

tiny: tiny.c csapp.o
	$(CC) $(CFLAGS) -o tiny tiny.c csapp.o $(LIB)

echo: echoclient.c csapp.o echo.o echoserveri.o
	$(CC) $(CFLAGS) -o echoclient echoclient.c csapp.o echo.o $(LIB)

echoserveri.o:
	$(CC) $(CFLAGS) -o echoserveri echoserveri.c csapp.o echo.o $(LIB)

echo.o:
	$(CC) $(CFLAGS) -c echo.c

csapp.o: csapp.c
	$(CC) $(CFLAGS) -c csapp.c

cgi:
	(cd cgi-bin; make)

clean:
	rm -f *.o tiny echoclient echoserveri *~
	(cd cgi-bin; make clean)

