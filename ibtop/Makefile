
CC 	= gcc
LIBS 	= -libmad -libumad -libnetdisc -lcurses

ibtop: ibtop.c fabric.h fabric.c
	$(CC) -o ibtop ibtop.c fabric.c

clean:
	-rm ibtop

