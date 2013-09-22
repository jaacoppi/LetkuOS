# A very simple Makefile and incomplete Makefile
# originally for LetkuOS

SRCDIR=src
INCDIR=include
SOURCES  := $(wildcard $(SRCDIR)/*.c)
#INCLUDES := $(wildcard $(INCDIR)/*.h)
OBJECTS  := $(SOURCES:.c=.o)
CC=tcc
CFLAGS=-nostdlib -I./include -Wall,Wl,-Ttext,0x100000, -static -DREVID="\"$(REVID)\""
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

bootstuff:
	@echo "Assembling $(BOOTFILE).."
	@$(AS) $(ASFLAGS) $(BOOTFILE) -o $(BOOTOBJ) 	# assembly

$(OBJECTS): $(SOURCES) #$(INCLUDES)
	@echo "Compiling C source files to object files.."
	$(CC) $(CFLAGS) -o $@ -c $*.c	# C


linking: bootstuff $(OBJECTS)
	@echo "Linking $(BOOTFILE) with C object files.."
	$(CC) $(CFLAGS) $(OBJECTS) $(BOOTOBJ) -o $(KERNELBIN)
	@echo ""
	@echo "Kernel compilation finished. Kernel at $(KERNELBIN)"


install: all
	@echo "Needing root access for loopdevice.."
	@sudo mount -tvfat -oloop=$(LOOPDEVICE),offset=32256 $(HDIMAGE) $(MOUNTPOINT)
	@sudo cp $(KERNELBIN) $(MOUNTPOINT)/boot/$(KERNELBIN)
	@sudo umount $(MOUNTPOINT)
	@echo Kernel image saved to $(HDIMAGE)

clean:
	@rm -f $(SRCDIR)/*.o

run: install
	@echo "Running bochs"
	@$(TESTING)/run_bochs

github: 
	git push -u origin master
