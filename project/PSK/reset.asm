; RESET.asm
; The reset vector in flash is located at 3F 7FF6.  (it can't be moved)
; A branch instruction located there takes us to the start of code.
; an offset of -32758 (800A) brings us to 3F000, which seems as nice of
; a place as any to locate our code.

; I couldn't seem to make it find the lable for c_int00, so I just
; hardcoded the opcode for the proper length.
 .sect ".flashreset"
 .ref	_c_int00 
 
; .word 0FFEFH	; Branch unconditionally
; .word 0800AH	; 	to 0x3F0000

	B 	_c_int00, UNC	;

;	LB 	_c_int00		;