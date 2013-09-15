# A very simple Makefile and incomplete Makefile
# originally for LetkuOS

SRCDIR=src
CC=tcc
CFLAGS=-nostdlib -I./include -Wall,Wl,-Ttext,0x100000, -static
AS=nasm
ASFLAGS=-f elf
LD=ld
LOOPDEVICE=/dev/loop0
HDIMAGE=LetkuOS.img
MOUNTPOINT=/mnt
KERNELBIN=LetkuOS.krn
TESTING=testing/
BOOTFILE=$(SRCDIR)/boot.asm
all: bootstuff sources linking

bootstuff:
	@echo "Assembling $(BOOTFILE).."
	@$(AS) $(ASFLAGS) $(BOOTFILE) 	# assembly

sources:
	@echo "Compiling C source files to object files.."
	@$(CC) $(CFLAGS) -c $(SRCDIR)/main.c -o $(SRCDIR)/main.o	# C
	@$(CC) $(CFLAGS) -c $(SRCDIR)/scrn.c -o $(SRCDIR)/scrn.o	# C

linking:
	@echo "Linking $(BOOTFILE) with C object files.."
	@$(CC) $(CFLAGS) $(SRCDIR)/boot.o $(SRCDIR)/main.o $(SRCDIR)/scrn.o -o $(KERNELBIN)
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
