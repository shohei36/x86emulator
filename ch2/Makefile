Z_TOOLS = ../tolset_p86/z_tools

CC = $(Z_TOOLS)/mingw32-gcc/bin/gcc
CFLAGS += -Wall

.PHONY : all

all : px86.exe helloworld.bin

px86.exe : main.o Makefile
	$(CC) -o $@ $<

main.o : main.c Makefile
	$(CC) $(CFLAGS) -c $<
	
helloworld.bin : helloworld.asm Makefile
	nasm -f bin -o $@ $<

