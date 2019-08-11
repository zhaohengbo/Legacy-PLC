//==========================================================================================
// Filename:		plc.h
//
// Description:		Power-Line Communication header file.
//					Contains compilation flags.
//					Contains typedefs for variable types
//					Contains structure definitions.
//					Contains function prototypes for all functions.
//					Contains global variables.
//					Contains global constants.
//					Contains macros.
//
// Copyright (C) 2004 Texas Instruments Incorporated
// Texas Instruments Proprietary Information
// Use subject to terms and conditions of TI Software License Agreement
//
// Revision History:	MOVED TO END OF FILE!
//==========================================================================================


#ifndef plc_h							// Header file guard
#define plc_h


//==========================================================================================
// Compilation Flags
//==========================================================================================


//==========================================================================================
// Global Type Definitions
//==========================================================================================


//==========================================================================================
// System #include files <filename.h>
// Application #includes "filename.h"
//==========================================================================================
//#ifdef __cplusplus
//	#include <cstdlib>		// C++ standard library
//#endif

//#include "DSP28_Device.h"	// DSP28 general file. device #includes, register definitions.
#include "prototypes.h"		// global prototype declarations.


//==========================================================================================
// Global Constants - General purpose constants
//==========================================================================================
#define SUCCESS			(0)			// Return code for success.

//==========================================================================================
// Global Constants - Related to hardware. Register bits, ...
//==========================================================================================
//#define	TX_MODE		1
//#define	RX_MODE		2


//==========================================================================================
// Bit masks for Flag Registers
//==========================================================================================


//==========================================================================================
// Command Specific constants.
//==========================================================================================
// Read Memory
#define RM_ADDR_MAX						(28)			// Max count in address mode
														//	(set by available parms)
// Write Memory
#define WM_ADDR_MAX						(14)			// Max count in address/enum mode


//==========================================================================================
// Macros
//==========================================================================================

								   
//==========================================================================================
// Global Variables declarations.  (matching definitions found in "vardefs.h")
//==========================================================================================

extern u16	T1PIntCount;		// EV Timer1 Period Interrupt counts
extern u16	txBitPending;		// Flag to indicate that a transmit bit is ready to be sent
extern u16	txBitNext;			// Value of next bit to be sent via PLC


extern u16	uFIROutIndex;				// Index into FIR output buffers
extern q16	qFIROutP[FIR_OUT_LEN];		// Primary FIR output buffer	
extern q16	qFIROutQ[FIR_OUT_LEN];		// Quadrature FIR output buffer
//extern q32	qlFIROut[FIR_OUT_LEN];	// FIR outputs: Primary in high word, Quadrature in low word
extern q16	qFIROutOld[FIR_OUT_LEN];	// Delayed copy of FIR output
extern q32	qlRxRawBuff[FIR_LEN];	// Array with most recent raw data
extern q32* pRxRaw;					// Pointer into RxRawBuff
extern u16	uCorrOutIndex;			// Index into correlation output array
extern q32	qlCorrOut[CORR_OUT_LEN];// Correlation output filter
extern q32	qlSumCorrOut;			// Sum of most recent period of correlation output filter
extern q16 qPreDetCntr;				// Number of negative values seen in the correlation output
//==========================================================================================
#endif									// End of header guard: #ifndef main_h


//==========================================================================================
// Revision History:
//
// 04/15/04	HEM		New file.
// 11/17/04	HEM		Removed rxBitPending.
// 11/19/04	HEM		Moved ADCIntCount to main.h.
//==========================================================================================






