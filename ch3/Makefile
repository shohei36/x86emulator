Z_TOOLS = ../tolset_p86/z_tools
CS = main.c emulator_function.c instruction.c modrm.c io.c
OBJS = main.o emulator_function.o instruction.o modrm.o io.o

# Optimaze Option
CFLAGOPT = 
CFLAGOPT += -O0

CC = $(Z_TOOLS)/mingw32-gcc/bin/gcc
CFLAGS =
CFLAGS += -Wall
CFLAGS += -g 
CFLAGS += $(CFLAGOPT)
CFLAGS += -Wall

.PHONY : all

all : main.exe modrm-test.bin call-test.bin select.bin

main.exe : $(OBJS) Makefile
	$(CC) -o $@ $(OBJS)

%.o : %.c Makefile
	$(CC) $(CFLAGS) -c $< 
	
modrm-test.bin : modrm-test.asm Makefile
	nasm -f bin -o $@ $<

call-test.bin : call-test.asm Makefile
	nasm -f bin -o $@ $<

select.bin : select.asm Makefile
	nasm -f bin -o $@ $<