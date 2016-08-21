
CC       = /usr/bin/gcc
CPPFLAGS = -I/usr/include/infiniband -I. -D_GNU_SOURCE -DPARALLEL=1
CCFLAGS  = -std=gnu99 -Wall -fPIC -O0 -ggdb -fopenmp
LD       = /usr/bin/gcc
LDFLAGS  = -std=gnu99 -Wall -fPIC -O0 -ggdb -fopenmp
LIBS     = -libmad -libumad -libnetdisc -libverbs
AR       = /usr/bin/ar

Q = @

default: all

all: libcommon.a ibutil001.exe ibutil002.exe ibutil003.exe ibutil004.exe ibutil006.exe ibutil007.exe ibutil008.exe ibutil009.exe ibutilpy.so

ibutil%.exe: ibutil%.o libcommon.a
	$(Q)$(LD) $(LDFLAGS) -o $@ $< $(LIBS) -L$(PWD) -lcommon
	@echo "LD $@"

ibutilpy.so: ibutilpy.c libcommon.a
	$(Q)$(CC) $(CPPFLAGS) $(CCFLAGS) -o $@ $< -shared $(LIBS) -L$(PWD) -lcommon
	@echo "CC $@"

libcommon.a: common.o adapter.o mad-handling.o ibutil.o error.o
	$(AR) rcs $@ $^
	@echo "AR $@"

%.o: %.c
	$(Q)$(CC) $(CPPFLAGS) $(CCFLAGS) -o $@ -c $<
	@echo "CC $@"

clean:
	-rm -f *.a *.o *.exe *.so

