
CC       = /usr/bin/gcc
CPPFLAGS = -I/usr/include/infiniband -I. -D_GNU_SOURCE -DPARALLEL=1
CCFLAGS  = -Wall -O0 -ggdb -fopenmp
LD       = /usr/bin/gcc
LDFLAGS  = -Wall -O0 -ggdb -fopenmp
LIBS     = -libmad -libumad -libnetdisc -libverbs
AR       = /usr/bin/ar

Q = @

default: all

all: libcommon.a ibutil001.exe ibutil002.exe ibutil003.exe

ibutil%.exe: ibutil%.o libcommon.a
	$(Q)$(LD) $(LDFLAGS) -o $@ $< $(LIBS) -L$(PWD) -lcommon
	@echo "LD $@"

libcommon.a: common.o adapter.o error.o
	$(AR) rcs $@ $^
	@echo "AR $@"

%.o: %.c
	$(Q)$(CC) $(CPPFLAGS) $(CCFLAGS) -o $@ -c $<
	@echo "CC $@"

clean:
	-rm -f *.a *.o *.exe

