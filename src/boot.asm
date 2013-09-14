;/* boot.asm
;*
;* get control from GRUB (or other bootloader,
;* loads stuff in order to be able to do everything else in C
;*
;*/
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Globals and externs
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

global _start           ; making entry point visible to linker

extern main            ; main is in main.c


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Multiboot (aka GRUB) -compliance
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; setting up the Multiboot header - see GRUB docs for details
MODULEALIGN equ  1<<0                   ; align loaded modules on page boundaries
MEMINFO     equ  1<<1                   ; provide memory map
FLAGS       equ  MODULEALIGN | MEMINFO  ; this is the Multiboot 'flag' field
MAGIC       equ    0x1BADB002           ; 'magic number' lets bootloader find the header
CHECKSUM    equ -(MAGIC + FLAGS)        ; checksum required

section .text
align 4
MultiBootHeader:
	dd MAGIC
	dd FLAGS
	dd CHECKSUM

; reserve initial kernel stack space 
STACKSIZE equ 0x4000                  ; 16k 

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Entry point start
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_start:
	mov esp, stack+STACKSIZE           ; set up the stack

	; push gives the boot parameters to main()
	; remember, the stack is read in reverse order, so parameter one
	; is now the magic number
	push eax                        ; pass Multiboot magic number
	push ebx                        ; pass Multiboot info structure
	call  main			; call kernel proper
;	TODO: print an error if main returns
	jmp _start				; if main returns, reboot
;	hlt                                ; halt machine should kernel 
;					;return

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Stack here
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

section .bss
align 32

stack:
   resb STACKSIZE                     ; reserve 16k stack on a quadword boundary
