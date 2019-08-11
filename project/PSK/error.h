//==========================================================================================
// Filename:		command.h
//
// Description:		Prototypes for MCU to DSP command functions.
//
// Copyright (C) 2000 - 2001 Texas Instruments Incorporated
// Texas Instruments Proprietary Information
// Use subject to terms and conditions of TI Software License Agreement
//
// Revision History:
// 05/31/02	EGO		Started file.  From jervis project.
// 06/20/02 EGO		Removed prototype for nonexistent functino PostError()
// 06/25/02 EGO		Enabled constants for read/write memory and calibrate.
// 06/26/02 EGO		Enabled Control Temp error codes.
// 06/26/02 EGO		Enabled DiagTrace error codes.
//					Removed TFA constants.
// 06/26/02 HEM		Added status and LED error codes.
//==========================================================================================


#ifndef error_h						// Header file guard
#define error_h


//==========================================================================================
// #Defined Constant Definitions
//==========================================================================================
// Command return codes not associated with a specific command.
#define ERR_UKNOWN_COMMAND				(0x0001)

// Read Memory command return codes
#define ERR_RM_INVALID_COUNT			(0x0011)	// Invalid count specified

// Write Memory command return codes
#define ERR_WM_INVALID_COUNT			(0x0022)

// Calibrate command return codes
#define ERR_CAL_INVALID_CHANNEL			(0x0030)
#define ERR_CAL_CAL_NOT_SUPPORTED		(0x003F)

// Control Temperature command return codes
//#define ERR_CTRL_INVALID_PARM			(0x0061)
#define ERR_CTRL_RANGE_EXCEEDED			(0x0062)

//// Constant Output command return codes
//#define ERR_OUT_INVALID_PARM			(0x0071)

// Diag Trace command return codes
#define ERR_TRACE_LIST_UNDEFINED		(0x0100)



// Channel Status Bit Masks and LEDs Error Codes
//   These bit masks are combined in uChanStatus[].
//   All except bit 0 will cause blinking of the red LED status registers and 
//   initiate damage prevention procedures.
#define		ERR_TEMP_RANGE	(1<<0)	// Bit 0: TEC Temperature not within operating range
#define		ERR_BIT1		(1<<1)	// Bit 1: Reserved 
#define 	ERR_MAXV		(1<<2)	// Bit 2: Maximum TEC control voltage exceeded
#define		ERR_MAXI		(1<<3)	// Bit 3: Maximum TEC current exceeded
#define		ERR_TH_OPEN		(1<<4)	// Bit 4: Thermistor open-circuit
#define		ERR_TH_SHORT	(1<<5)	// Bit 5: Thermistor short-circuit
#define		ERR_DRV_MAXI	(1<<6)	// Bit 6: DRV592 fault 0 - Current > 4A
#define		ERR_DRV_LOWV	(1<<7)	// Bit 7: DRV592 fault 1 - Voltage < 2.8V
#define		ERR_DRV_TEMP	(1<<8)	// Bit 8: DRV592 fault 2 - Driver junction temp > 130 C
#define		ERR_BIT9		(1<<9)	// Bit 9: Available
#define		ERR_BIT10		(1<<10)	// Bit 10: Available
#define		ERR_BIT11		(1<<11)	// Bit 11: Available
#define		ERR_BIT12		(1<<12)	// Bit 12: Available
#define		ERR_BIT13		(1<<13)	// Bit 13: Available
#define		ERR_BIT14		(1<<14)	// Bit 14: Available
#define		ERR_BIT15		(1<<15)	// Bit 15: Available


//==========================================================================================
// Function Prototypes
//==========================================================================================


//==========================================================================================
#endif									// End of header guard: #ifndef error_h


