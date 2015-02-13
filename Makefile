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
	@echo "\nConverting elf to uarm executable"
	$(CONVERTER) $(CONVFLAGS) p1test
	
p1test: pcb.o asl.o p1test.o
	@echo "\nLinking all modules"
	$(LD) $(LDFLAGS) $(EXECUTABLE) $(CRTSO) $(LIBUARM) $(sourcedir)pcb.o $(sourcedir)asl.o $(sourcedir)p1test.o
	
pcb.o: $(sourcedir)pcb.c
	@echo "\nCompiling pcb module"
	$(CC) $(CFLAGS) $(sourcedir)pcb.o $(sourcedir)pcb.c
	
asl.o: $(sourcedir)asl.c
	@echo "\nCompiling asl module"
	$(CC) $(CFLAGS) $(sourcedir)asl.o $(sourcedir)asl.c
	
p1test.o: $(sourcedir)p1test.c
	@echo "\nCompiling p1test module"
	$(CC) $(CFLAGS) $(sourcedir)p1test.o $(sourcedir)p1test.c
	
clean:
	@echo "\nCleaning object files"
	rm -rf $(sourcedir)*.o
