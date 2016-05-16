
CC       = /usr/bin/gcc
CPPFLAGS = -I. -D_GNU_SOURCE
CCFLAGS  = -Wall -O0 -ggdb -I/usr/include/infiniband
LD       = /usr/bin/gcc
LDFLAGS  = -Wall -O0 -ggdb 
LIBS     = -libmad -libumad -libnetdisc -libverbs
AR       = /usr/bin/ar

Q = @

default: all

all: libcommon.a ibutil001.exe ibutil002.exe

ibutil%.exe: ibutil%.o libcommon.a
	$(Q)$(LD) $(LDFLAGS) -o $@ $< $(LIBS) -L$(PWD) -lcommon
	@echo "LD $@"

libcommon.a: common.o adapter.o error.o
	ar rcs $@ $^
	@echo "AR $@"

%.o: %.c
	$(Q)$(CC) $(CPPFLAGS) $(CCFLAGS) -o $@ -c $<
	@echo "CC $@"

clean:
	-rm -f *.a *.o *.exe

