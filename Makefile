# A very simple Makefile and incomplete Makefile
# originally for LetkuOS

SRCDIR=src
CC=tcc
CFLAGS=-nostdlib -Wall,Wl,-Ttext,0x100000, -Iinclude/ -static
AS=nasm
ASFLAGS=-f elf
LD=ld
LOOPDEVICE=/dev/loop0
HDIMAGE=LetkuOS.img
MOUNTPOINT=/mnt
KERNELBIN=LetkuOS.krn

all:
	$(AS) $(ASFLAGS) $(SRCDIR)/boot.asm 	# assembly
	$(CC) -c $(SRCDIR)/main.c -o $(SRCDIR)/main.o	# C
	$(CC) $(CFLAGS) $(SRCDIR)/boot.o $(SRCDIR)/main.o -o $(KERNELBIN)

install:
	@echo "Needing root access for loopdevice.."
	@sudo mount -tvfat -oloop=$(LOOPDEVICE),offset=32256 $(HDIMAGE) $(MOUNTPOINT)
	@sudo cp $(KERNELBIN) $(MOUNTPOINT)/boot/$(KERNELBIN)
	@sudo umount $(MOUNTPOINT)
	@echo Kernel image saved to $(HDIMAGE)

clean:
	@rm -f $(SRCDIR)/*.o
