;; loop.asm - a simple loop for wasting some seconds to see if loading userspace programs work..

section .text
	global _start

_start:
;	mov ecx, 32255
	mov ecx, 999999999
loophere:
	loop loophere


; no segfault with linux, please..
mov eax,1            ; The system call for exit (sys_exit)
mov ebx,0            ; Exit with return code of 0 (no error)
;int 80h
int 14h	;debug int 14h in LetkuOS
