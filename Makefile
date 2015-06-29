#Makefile

includedir = include/
sourcedir = source/

CC = arm-none-eabi-gcc
CFLAGS = -mcpu=arm7tdmi -I $(includedir) -I /usr/include/uarm -c -o
LD = arm-none-eabi-ld
EXECUTABLE = initial
CRTSO = /usr/include/uarm/crtso.o
LIBUARM = /usr/include/uarm/libuarm.o
LDFLAGS = -T /usr/include/uarm/ldscripts/elf32ltsarm.h.uarmcore.x  -o
CONVERTER = elf2uarm
CONVFLAGS = -k

all: kernel.core.uarm

kernel.core.uarm: initial
	@echo "Converting elf to uarm executable"
	$(CONVERTER) $(CONVFLAGS) initial

initial: initial.o pcb.o asl.o scheduler.o exceptions.o interrupts.o syscall.o bitmap.o p2test.o $(includedir)pcb.h $(includedir)asl.h $(includedir)const.h $(includedir)listx.h $(includedir)scheduler.h $(includedir)exceptions.h $(includedir)interrupts.h $(includedir)syscall.h $(includedir)bitmap.h
	@echo "Linking modules"
	$(LD) $(LDFLAGS) $(EXECUTABLE) $(CRTSO) $(LIBUARM) $(sourcedir)initial.o $(sourcedir)pcb.o $(sourcedir)asl.o $(sourcedir)scheduler.o $(sourcedir)exceptions.o $(sourcedir)interrupts.o $(sourcedir)p2test.o $(sourcedir)syscall.o $(sourcedir)bitmap.o

initial.o: $(sourcedir)initial.c $(includedir)pcb.h $(includedir)asl.h $(includedir)const.h $(includedir)listx.h $(includedir)scheduler.h $(includedir)exceptions.h $(includedir)interrupts.h
	@echo "Compiling initial module"
	$(CC) $(CFLAGS) $(sourcedir)initial.o $(sourcedir)initial.c

scheduler.o: $(sourcedir)scheduler.c $(includedir)syscall.h $(includedir)initial.h $(includedir)pcb.h $(includedir)const.h $(includedir)types.h
	@echo "Compiling scheduler module"
	$(CC) $(CFLAGS) $(sourcedir)scheduler.o $(sourcedir)scheduler.c

exceptions.o: $(sourcedir)exceptions.c $(includedir)syscall.h $(includedir)const.h
	@echo "Compiling exceptions module"
	$(CC) $(CFLAGS) $(sourcedir)exceptions.o $(sourcedir)exceptions.c

interrupts.o: $(sourcedir)interrupts.c $(includedir)scheduler.h $(includedir)initial.h $(includedir)const.h $(includedir)bitmap.h $(includedir)syscall.h
	@echo "Compiling interrupts module"
	$(CC) $(CFLAGS) $(sourcedir)interrupts.o $(sourcedir)interrupts.c

syscall.o: $(sourcedir)syscall.c $(includedir)pcb.h $(includedir)asl.h $(includedir)listx.h $(includedir)const.h $(includedir)bitmap.h $(includedir)scheduler.h $(includedir)interrupts.h
	@echo "Compiling syscall module"
	$(CC) $(CFLAGS) $(sourcedir)syscall.o $(sourcedir)syscall.c

bitmap.o: $(sourcedir)bitmap.c
	@echo "Compiling bitmap library"
	$(CC) $(CFLAGS) $(sourcedir)bitmap.o $(sourcedir)bitmap.c

p2test.o: $(sourcedir)p2test.c $(includedir)listx.h $(includedir)pcb.h
	@echo "Compiling p2test module"
	$(CC) $(CFLAGS) $(sourcedir)p2test.o $(sourcedir)p2test.c

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
	rm -rf $(sourcedir)*.o *.uarm initial
