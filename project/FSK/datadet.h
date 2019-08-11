//==========================================================================================
// Filename:		datadet.h
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
// 01/25/04	Hagen		New file.
// 07Mar05	Hagen		added extern reference to uTxPrecodeTable[]
//==========================================================================================


#ifndef datadet_h							// Header file guard
#define datadet_h


//==========================================================================================
// Compilation Flags
//==========================================================================================


//==========================================================================================
// Global Type Definitions
//==========================================================================================


//==========================================================================================
// System #include files <filename.h>
// Application #includes "filename.h"

#include "DSP280x_Device.h"	// DSP28 general file. device #includes, register definitions.
#include "prototypes.h"		// global prototype declarations.

#include "main.h"			// global var declarations.


//---- Word size ---------------------------------
#define 	BYTE_LEN         		8
#define  	BYTE_MSB            	0x0080U
#define 	WORD_MSB     			0x8000U

#define		PI						3.14159265	
#define		BITSYNC_LEN				24
#define		CODEWORD_LEN			11
#define		WORDSYNC_LEN			CODEWORD_LEN
#define		POSTAMBLE_LEN			(CODEWORD_LEN*2)

#define 	BITSYNC_PATTERN			0xAAAA		// look for these data patterns
#define 	WORDSYNC_PATTERN		0x567B
#define 	WORDSYNC_PAT_NEG		(0xFFFF ^ WORDSYNC_PATTERN)
#define 	EOP_PATTERN				0x0733

// Arefeen added on 062705
u16			bitPhase;
u16			polarity;
u16			hystCnt;

//---- complex array definition ------------------------------
typedef struct
{
	s16				re;
	u16				im;
}	iCplx;				// basic complex data element

typedef struct 
{
	iCplx			*c;
	u16				len;
}	iCplxPtr;			// container for complex data

typedef struct 
{
	s16				*r;
	s16				*beg;
	u16				len;
}	iRealPtr;			// container for real data

typedef struct 
{
	s16				*r;
	s16				*start;
	s16				*end;
	u16				len;
	s16				offset;
}	iCircPtr;			// container for real data

typedef struct 
{
	u16				*r;
	u16				len;
}	uRealPtr;

typedef struct 
{
	u8				*r;
	int				len;
}	bytePtr;


/*==================================================================
enumerated types
==================================================================*/
typedef	enum
{					IdleState, 
					TxMsgReceived, 
					PreambleDetected,
					FrameAligned,
					RxMsgReceived,
					RxMsgDecoded, 
					PreambleMaybe
}   modemStateType;

typedef	enum
{					Det_Pre_Mode, 
					Det_Sync_Mode,
					Det_Data_Mode
}   rxModeType;

typedef	enum
{					AgcIdle, 
					AgcPreamble, 
					AgcHold 
}   agcStateType;

typedef	enum
{					diagU16, 
					diags16,
					diagICPLX
}	diagType;


/*==============================================================
	Global Variables
===============================================================*/
//----Global vars declared elseware---------------------------------
extern pllControl		pll;			// global vars that control pll opperation

extern u16 				uRxModeCount; 	// how long in uRxMode


extern const u16 	uTxPrecodeTable[256];  //used to calc parity for received word 

//----Global vars used in detData---------------------------------
#define	PLL_FIR_LEN		(2*7)			//
s16			sinBuf[PLL_FIR_LEN];		// PLL fir




//----sine table ------------------------------------------------
#define ST_SCALE				2		// decrease resolution this much from full scale
										//    (saves doing shift in mainline code)
#define SINTABLE_LEN			64		// 
const s16	sinTable[] = 				// cosine and sine generator table for digital PLL
{										// get sine by indexing into the table by 16
     32767>>ST_SCALE,  32610>>ST_SCALE,  32138>>ST_SCALE,  31357>>ST_SCALE,  
     30274>>ST_SCALE,  28899>>ST_SCALE,  27246>>ST_SCALE,  25330>>ST_SCALE,
     23170>>ST_SCALE,  20788>>ST_SCALE,  18205>>ST_SCALE,  15447>>ST_SCALE,  
     12540>>ST_SCALE,   9512>>ST_SCALE,   6393>>ST_SCALE,   3212>>ST_SCALE,
         0>>ST_SCALE,  -3212>>ST_SCALE,  -6393>>ST_SCALE,  -9512>>ST_SCALE, 
    -12540>>ST_SCALE, -15447>>ST_SCALE, -18205>>ST_SCALE, -20788>>ST_SCALE,
    -23170>>ST_SCALE, -25330>>ST_SCALE, -27246>>ST_SCALE, -28899>>ST_SCALE, 
    -30274>>ST_SCALE, -31357>>ST_SCALE, -32138>>ST_SCALE, -32610>>ST_SCALE,
    -32767>>ST_SCALE, -32610>>ST_SCALE, -32138>>ST_SCALE, -31357>>ST_SCALE, 
    -30274>>ST_SCALE, -28899>>ST_SCALE, -27246>>ST_SCALE, -25330>>ST_SCALE,
    -23170>>ST_SCALE, -20788>>ST_SCALE, -18205>>ST_SCALE, -15447>>ST_SCALE, 
    -12540>>ST_SCALE,  -9512>>ST_SCALE,  -6393>>ST_SCALE,  -3212>>ST_SCALE,
        -0>>ST_SCALE,  +3212>>ST_SCALE,  +6393>>ST_SCALE,  +9512>>ST_SCALE, 
    +12540>>ST_SCALE, +15447>>ST_SCALE, +18205>>ST_SCALE, +20788>>ST_SCALE,
    +23170>>ST_SCALE, +25330>>ST_SCALE, +27246>>ST_SCALE, +28899>>ST_SCALE, 
    +30274>>ST_SCALE, +31357>>ST_SCALE, +32138>>ST_SCALE, +32610>>ST_SCALE,
     32767>>ST_SCALE,  32610>>ST_SCALE,  32138>>ST_SCALE,  31357>>ST_SCALE, 
     30274>>ST_SCALE,  28899>>ST_SCALE,  27246>>ST_SCALE,  25330>>ST_SCALE,
     23170>>ST_SCALE,  20788>>ST_SCALE,  18205>>ST_SCALE,  15447>>ST_SCALE,  
     12540>>ST_SCALE,   9512>>ST_SCALE,   6393>>ST_SCALE,   3212>>ST_SCALE
};

	
//----AGC gain constants ------------------------------------------------
#define AGC_SCALE				5		// left shift by this after multiplying ADCsignal with gainTable[]
#define GAIN_TABLE_SCALE		4		// sGainIndex is left shifted by this to index gainTable
#define GAIN_TABLE_UNITY_IX		5		// gainTable index that represents a gain of 1.0 (actually 1.03)
#define GAIN_TABLE_LEN			40		// length of gainTable

const s16 gainTable[GAIN_TABLE_LEN] = 	// with AGC_SCALE set to 5,
{										// the gain ranges from 0.5 to 43.4 in 1.25 dB steps 
    16,     19,     21,     25,     29,     33,     38,     44, 
    51,     59,     68,     78,     90,    104,    120,    139, 
   160,    185,    214,    247,    285,    329,    380,    439, 
   507,    586,    676,    781,    902,   1041,   1203,   1389,
  1604,   1852,   2139,   2470,   2852,   3293,   3803,   4392
};






//==============================================================
//	function prototypes for this module
//===============================================================
s16 	runPLL(s16 signal);
//void 	reset_to_BitSync(void);




/*==========================================================================================
  Function:		SaveTrace()

  Description: 	This function stores data to the trace buffer 

  Revision History:
  06/07/04	HEM		New Function.  
  02/14/05	Hagen	Added to datadet.h
==========================================================================================*/
#if TRACE_BUF_LEN > 0

	#ifdef DSP_COMPILE
	void inline SaveTrace(u16 uData)
	{
		upTraceBuffer[uTraceIndex++] = uData;
	
		if (uTraceIndex >= (((u16)(TRACE_BUF_LEN/4))*4))
			uTraceIndex = 0;
	}
	#endif
	
	// for MEX_COMPILE SaveTrace() is a macro defined in psk_macros.h
	
#endif






#endif    // file guard
