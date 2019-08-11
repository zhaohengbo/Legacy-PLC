;+==========================================================================================
; Filename:		subs.asm
;
; Description:	Assembly function subroutines used to perform low level routines that
;				aren't possible from C.
;
; Copyright (C) 2002 Texas Instruments Incorporated
; Texas Instruments Proprietary Information
; Use subject to terms and conditions of TI Software License Agreement
;
; Revision History:
; 06/24/02	EGO		Started file.  From jervis project.
;===========================================================================================

;===========================================================================================
; Function:		ReadProg
;
; Description: 	This function is used to read a single word from program space.
;				The single argument both in and out is passed in AL.
;
; Revision History:
; 06/24/02	EGO		Started file.
;===========================================================================================
 .def _ReadProg

_ReadProg:
	ASP								;Align stack pointer for 32 bit store	NEEDED?????
	MOVL	*SP++, XAR7				;Context save. Push current AR7 contents onto stack.
	MOVB	XAR7, #0				;Clear uppoer word of AR7
	MOV		AR7, AL					;Put address to read into lower word of AR7 for PREAD below.
	PREAD	AL, *XAR7				;Get desired program word. AL for return to C.
	MOVL	XAR7, *--SP				;Context resotre.
	NASP							;Undo ASP from above.
	XRETC	UNC						;Return leaving requested data in AL.


;===========================================================================================
; Function:		WriteProg
;
; Description: 	This function is used to write a single word to program space.
;				On calling the address (first argument) is passed in AL,
;				the value to write (second argument) is passed in AH.
;				There is no return value.
;
; 06/24/02	EGO		Started file.
;===========================================================================================
 .def _WriteProg

_WriteProg:
	ASP								;Align stack pointer for 32 bit store	NEEDED?????
	MOVL	*SP++, XAR7				;Context save. Push current AR7 contents onto stack.
	MOVB	XAR7, #0				;Clear uppoer word of AR7
	MOV		AR7, AL					;Put address to read into lower word of AR7 for PWRITE below.
	PWRITE	*XAR7, AH				;Write data to address in program space.
	MOVL	XAR7, *--SP				;Context resotre.
	NASP							;Undo ASP from above.
	XRETC	UNC						;Back to the main program.



;===========================================================================================
; Function:		Sat16
;
; Description: 	This function saturates a value to +/- 32767.  
;
; Input:		ACC	= Arg1	= Value
;
; Output:		ACC
;
; Modifies:		XAR4 (not a problem if called from 'C')
;
; Revision History:
; 07/18/02  HEM		New function.
;===========================================================================================
 .def _Sat16

_Sat16:								; First argument already in ACC
	MOVL 	XAR4, #0x7FFF			; Set XAR4 = Upper limit = +32767
	MINL 	ACC, @XAR4 				; Set ACC = MIN(ACC, +32767)
	NEG 	ACC						; Negate ACC
	MINL 	ACC, @XAR4 				; Set ACC = MIN(-ACC, +32767) == MAX(ACC,-32767)
	NEG		ACC						; Negate ACC back to its proper sign
	LRETR							; Return with saturated value in ACC


;===========================================================================================
; Function:		Saturate
;
; Description: 	This function saturates a value to a specified range.
;
; Input:		ACC   = Arg1 = Value
;				SP[4] = Arg2 = Lower Limit
;				SP[2] = Arg3 = Upper Limit
;
; Output:		ACC 
;
; Revision History:
; 07/18/02  HEM		New function
;===========================================================================================
 .def _Saturate

_Saturate:							; First argument already in ACC
	MINL	ACC, *-SP[2]			; Set ACC = MIN(ACC, upper limit)
	MAXL	ACC, *-SP[4]			; Set ACC = MAX(ACC, lower limit)
	LRETR							; Return with saturated value in ACC

;??? Might have the two arguments swapped???


;;===========================================================================================
;; Function:		Max16
;;
;; Description: 	This function the larger of two 16-bit inputs.
;;
;; Input:		AL = Arg1
;;				AH = Arg2
;;
;; Output:		AL 
;;
;; Revision History:
;; 10/19/04  HEM		New function
;;===========================================================================================
; .def _Max16
;_Max16:
;	MAX		AL, @AH					; Set AL = MAX(Arg1, Arg2)
;	LRETR
;	
;	
;;===========================================================================================
;; Function:		Min16
;;
;; Description: 	This function the smaller of two 16-bit inputs.
;;
;; Input:		AL = Arg1
;;				AH = Arg2
;;
;; Output:		AL 
;;
;; Revision History:
;; 10/19/04  HEM		New function
;;===========================================================================================
;	.def _Min16
;_Min16:
;	MIN		AL, @AH					; Set AL = MIN(Arg1, Arg2)
;	LRETR



 .def	_SmoothADCResults
AdcRegs_ADCRESULT0	.set	7108H
; OVERSAMPLE_RATE		.set	6
OVERSAMPLE_RATE		.set	4	;IMPORTANT!  This value must match value assigned in sensor.c
;===========================================================================================
; Function:		SmoothADCResults
;
; Description: 	This function calculates the average of the ADC readings, after discarding
;				the highest and lowest readings.
;
; Prototype:	u16 SmoothADCResults (void)
;
; Input:		None
;
; Output:		ACC = Smoothed results
;
; Revision History:
; 10/19/04  HEM		New function
;===========================================================================================
_SmoothADCResults:

	ADDB	SP,  #OVERSAMPLE_RATE		; Reserve space on stack for ADC results
	SETC	SXM							; Turn on sign-extension mode (signed numbers)

	; --- Copy ADC results onto stack.  Convert them to signed values. ---
	; Values on stack are stored in reverse order, but that has no effect on final answer.
	MOVZ	AR5, @SP					; XAR5 = Pointer into signed ADC results array on stack
	MOVL	XAR6, #AdcRegs_ADCRESULT0	; XAR6 = Pointer into raw ADC results array
	MOV		AR4, #(OVERSAMPLE_RATE-1)	; AR4 = Loop counter - Init it with uCount-1
SAR_Loop:
		MOV		ACC, *XAR6++			; Fetch raw value from ADC results reg
		SUB		ACC, #8000H				; Convert to signed
		MOV		*--XAR5, ACC 			; Store on stack
	BANZ	SAR_Loop, AR4--				; Bottom of loop		

	; --- Find largest value in ADC Results array ---
	MOV		AH, #-8000H					; Start AH at large negative value
	MOVZ	AR5, @SP					; XAR5 = Pointer into signed ADC results array on stack
	RPT		#(OVERSAMPLE_RATE-1)		;
||	MAX		AH, *--XAR5 				; AH = Largest sample in ADC results array
	MOV		PH, @AH						; Store largest sample in PH

	; --- Find smallest value in ADC Results array ---
	MOV		AL, #7FFFH					; Start AL at  large positive value
	MOVZ	AR5, @SP					; XAR5 = Pointer into signed ADC results array on stack
	RPT		#(OVERSAMPLE_RATE-1)		;
||	MIN		AL, *--XAR5 				; AL = Smallest sample in ADC results array
	MOV		PL, @AL						; Store smallest sample in PL

	; --- Calculate sum of ADC Results array ---
	MOV		ACC, #0
	MOVZ	AR5, @SP					; XAR5 = Pointer into signed ADC results array on stack
	RPT		#(OVERSAMPLE_RATE-1)
||	ADD 	ACC, *--XAR5				; ACC = Running Sum

	SUB		ACC, @PH					; Subtract largest sample
	SUB		ACC, @PL					; Subtract smallest sample

	; --- Divide by (OVERSAMPLE_RATE-2)	---
	.if	OVERSAMPLE_RATE == 3
	  									; Divide by (3-2)
	.elseif OVERSAMPLE_RATE == 4	
	SFR		ACC, 1						; Divide by (4-2)
	.elseif OVERSAMPLE_RATE == 6
	SFR		ACC, 2						; Divide by (6-2)
	.else
	.emsg "Invalid value for OVERSAMPLE_RATE"	
	.endif
	
	SUBB	SP,  #OVERSAMPLE_RATE		; Restore stack pointer
	LRETR




