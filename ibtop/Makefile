
CC 	= gcc
CFLAGS	= -std=gnu99 -I/usr/include/infiniband -Wall
LIBS 	= -libmad -libumad -libnetdisc -lcurses -lm

ibtop: ibtop.c fabric.h fabric.c log.c
	$(CC) $(CFLAGS) -o ibtop ibtop.c fabric.c log.c $(LIBS)

clean:
	-rm ibtop

