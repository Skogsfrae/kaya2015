#Makefile

includedir = include/
sourcedir = source/

CC = arm-none-eabi-gcc
CFLAGS = -mcpu=arm7tdmi -I $(includedir) -I /usr/include/uarm -c -o
LD = arm-none-eabi-ld
EXECUTABLE = p1test
CRTSO = /usr/include/uarm/crtso.o
LIBUARM = /usr/include/uarm/libuarm.o
LDFLAGS = -T /usr/include/uarm/ldscripts/elf32ltsarm.h.uarmcore.x  -o
CONVERTER = elf2uarm
CONVFLAGS = -k

all: kernel.core.uarm

kernel.core.uarm: p1test
	@echo "Converting elf to uarm executable"
	$(CONVERTER) $(CONVFLAGS) p1test

p1test: pcb.o asl.o p1test.o $(includedir)listx.h $(includedir)asl.h $(includedir)pcb.h
	@echo "Linking all modules"
	$(LD) $(LDFLAGS) $(EXECUTABLE) $(CRTSO) $(LIBUARM) $(sourcedir)pcb.o $(sourcedir)asl.o $(sourcedir)p1test.o

pcb.o: $(sourcedir)pcb.c $(includedir)listx.h $(includedir)types.h
	@echo "Compiling pcb module"
	$(CC) $(CFLAGS) $(sourcedir)pcb.o $(sourcedir)pcb.c

asl.o: $(sourcedir)asl.c $(includedir)listx.h $(includedir)types.h $(includedir)pcb.h
	@echo "Compiling asl module"
	$(CC) $(CFLAGS) $(sourcedir)asl.o $(sourcedir)asl.c

p1test.o: $(sourcedir)p1test.c
	@echo "Compiling p1test module"
	$(CC) $(CFLAGS) $(sourcedir)p1test.o $(sourcedir)p1test.c

clean:
	@echo "Cleaning object files"
	rm -rf $(sourcedir)*.o *.uarm p1test
