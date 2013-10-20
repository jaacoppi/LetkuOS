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
BOOTFILE=$(SRCDIR)/boot.asm
BOOTOBJ=$(SRCDIR)/boot.o
REVID=$(shell date)


all: linking

beginning:
	@echo "Starting kernel compilation..."
	@echo "##############################"

bootstuff:
	@echo "Assembling $(BOOTFILE).."
	@$(AS) $(ASFLAGS) $(BOOTFILE) -o $(BOOTOBJ) 	# assembly

$(OBJECTS): $(SOURCES) #$(INCLUDES)
	@echo "Compiling C source file $@"
	@$(CC) $(CFLAGS) -o $@ -c $*.c	# C


linking: beginning bootstuff $(OBJECTS)
	@echo "Linking $(BOOTFILE) with C object files.."
	@$(LD) -T $(SRCDIR)/linker.ld -o $(KERNELBIN) $(BOOTOBJ) $(OBJECTS) #-L$(LIBGCC)
#	$(CC) $(CFLAGS) $(OBJECTS) $(BOOTOBJ) -o $(KERNELBIN)
	@echo ""
	@echo "############################################################"
	@echo "# Kernel compilation finished. Kernel saved to $(KERNELBIN) #"
	@echo "############################################################"


install: all
	@echo "Needing root access for loopdevice.."
	@sudo mount -tvfat -oloop=$(LOOPDEVICE),offset=32256 $(HDIMAGE) $(MOUNTPOINT)
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
