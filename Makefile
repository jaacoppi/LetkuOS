# A very simple Makefile and incomplete Makefile
# originally for LetkuOS

SRCDIR=src
CLIBDIR=$(SRCDIR)/clib
INCDIR=include
SOURCES  := $(wildcard $(SRCDIR)/*.c $(CLIBDIR)/*.c)
#INCLUDES := $(wildcard $(INCDIR)/*.h)
OBJECTS  := $(SOURCES:.c=.o)
CC=gcc
CFLAGS=-Wall -Wextra -I./include -nostartfiles  -fno-builtin -nostdinc \
        -nodefaultlibs -nostdlib -std=c99 -Wno-main \
	-static -DREVID="\"$(REVID)\""

AS=nasm
ASFLAGS=-f elf
LD=ld
LOOPDEVICE=/dev/loop0
HDIMAGE=LetkuOS.img
MOUNTPOINT=/mnt
KERNELBIN=LetkuOS.krn
TESTING=testing/
BOOTFILE=$(SRCDIR)/boot.s
BOOTOBJ=$(SRCDIR)/boot.o
REVID=$(shell date)

## testing userspace programs
LOOP=userprog/loop.c

all: beginning $(BOOTOBJ) $(OBJECTS)
	@echo "Linking $(BOOTFILE) with C object files.."
	@$(LD) -T $(SRCDIR)/linker.ld -o $(KERNELBIN) $(BOOTOBJ) src/interrupts.o $(OBJECTS) #-L$(LIBGCC)
	@echo ""
	@echo "############################################################"
	@echo "# Kernel compilation finished. Kernel saved to $(KERNELBIN) #"
	@echo "############################################################"


beginning:
	@echo "Starting kernel compilation..."
	@echo "##############################"

$(BOOTOBJ):
	@echo "Assembling $(BOOTFILE).."
	@$(AS) $(ASFLAGS) $(BOOTFILE) -o $(BOOTOBJ) 	# assembly
	@$(AS) $(ASFLAGS) src/interrupts.s -o src/interrupts.o	 	# assembly

$(OBJECTS): $(SOURCES) #$(INCLUDES)
	@echo "Compiling C source file $@"
	@$(CC) $(CFLAGS) -o $@ -c $*.c	# C

userprog: $(LOOP)
	@$(CC) $(CFLAGS) -o userprog/loop.o -c userprog/loop.c	# loop
	@$(LD) -T userprog/linker.ld -o userprog/loop userprog/loop.o

install: all
	@echo "Needing root access for loopdevice.."
	@sudo mount -tvfat -oloop=$(LOOPDEVICE),offset=1048576 $(HDIMAGE) $(MOUNTPOINT)
	@sudo cp $(KERNELBIN) $(MOUNTPOINT)/boot/$(KERNELBIN)
	@sudo umount $(MOUNTPOINT)
	@echo Kernel image saved to $(HDIMAGE)

clean:
	@rm -f $(SRCDIR)/*.o
	@rm -f $(CLIBDIR)/*.o

run: install
	@echo "Running bochs"
	@$(TESTING)/run_bochs

github: 
	git push -u origin master

