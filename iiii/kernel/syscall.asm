
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

INT_VECTOR_SYS_CALL equ 0x90
_NR_get_ticks       equ 0
_NR_write	    equ 1
; Added by lionel
_NR_process_sleep	equ 2
_NR_disp_str		equ 3
_NR_disp_color_str	equ 4
_NR_sem_p	equ 5
_NR_sem_v	equ 6


; 导出符号
global	get_ticks
global	write
global	user_process_sleep
global	user_disp_str
global	user_disp_color_str
global	user_sem_p
global	user_sem_v


bits 32
[section .text]

; ====================================================================
;                              get_ticks
; ====================================================================
get_ticks:
	mov	eax, _NR_get_ticks
	int	INT_VECTOR_SYS_CALL
	ret

; ====================================================================================
;                          void write(char* buf, int len);
; ====================================================================================
write:
        mov     eax, _NR_write
        mov     ebx, [esp + 4]
        mov     ecx, [esp + 8]
        int     INT_VECTOR_SYS_CALL
        ret

;Added by lionel
;PUBLIC void user_process_sleep(int milli_sec);
user_process_sleep:
		mov     eax, _NR_process_sleep
        mov     ebx, [esp + 4]
        int     INT_VECTOR_SYS_CALL
        ret
		
;PUBLIC void user_disp_str(char *str);
user_disp_str:
		mov     eax, _NR_disp_str
        mov     ebx, [esp + 4]
        int     INT_VECTOR_SYS_CALL
        ret


;PUBLIC void user_disp_color_str(char * info, int color);
user_disp_color_str:
		mov     eax, _NR_disp_color_str
        mov     ebx, [esp + 4]
        mov     ecx, [esp + 8]
        int     INT_VECTOR_SYS_CALL
        ret

;PUBLIC void user_sem_p(char *name);
user_sem_p:
		mov     eax, _NR_sem_p
        mov     ebx, [esp + 4]
        int     INT_VECTOR_SYS_CALL
        ret

;PUBLIC void user_sem_v(char *name);
user_sem_v:
		mov     eax, _NR_sem_v
        mov     ebx, [esp + 4]
        int     INT_VECTOR_SYS_CALL
        ret