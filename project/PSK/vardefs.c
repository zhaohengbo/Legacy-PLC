//==========================================================================================
// Filename:		vardefs.c
//
// Description:		Contains global variable definitions and function InitializeGlobals()
//					which is used to initialize variables which require it.
//
//					Note extern declarations are in main.h which is included by every
//					file.  Variable descriptions are given there (and not duplicated here).
//
// Copyright (C) 2002 Texas Instruments Incorporated
// Texas Instruments Proprietary Information
// Use subject to terms and conditions of TI Software License Agreement
//
// Revision History: 	Moved to end of file.
//==========================================================================================

#include "main.h"
#include "version.h"				// VERSION_NUMBER definition.
#include <string.h>					// contains memset()

/*
// Local variable - hand-placed for the tester in a known location.
#ifdef __cplusplus			// "C++"
#pragma DATA_SECTION("build_info");
u16	upBuildInfo[4];
#else						// "C"
#pragma DATA_SECTION(upBuildInfo, "build_info");
u16	upBuildInfo[4];
#endif
*/

// Variables declared in main.h
u16	uVersion[3] = {0, 0, 0};
u16	uMyAddress = 0;
u16	uDestAddress = 0;
u16 uSampleNumber;
u16 uStartFlag = 0;					// Stagger-start control flag. Init = all off.
volatile u32	ulTimerIntCounter;	// Updated by periodic timer ISR, so make it volatile

u16	upCommand[COMMAND_PARMS];
	asm("upCommand_DIM1 .set COMMAND_PARMS");
u16	upSerialCommand[COMMAND_PARMS];
	asm("upSerialCommand_DIM1 .set COMMAND_PARMS");

u16	uCommandPending;
u16	uCommandActive;
//u16	uCalibActive;
//u16 uSampleRate;


u16	uTxMsgPending = 0;					// Flag: Transmit message waiting to be sent
u16	uRxMsgPending = 0;					// Flag: Received message waiting to be processed

// Variables declared in sensor.h

u16	uADCTimeoutCnt;

u16	uFloodInterval = TINTS_PER_SEC;	// Number of TINT interrupts between fake PLC messages
//u16	uFloodInterval = 0;				// Number of TINT interrupts between fake PLC messages

u16	uFadeInterval;					// Number of TINT interrupts between lamp fader steps
u8	ubLampDTR;						// Lamp Controller Data Transfer Register
u8	ubLampIntensityTarget;			// Lamp Intensity Target Setting
u8	ubLampIntensity;				// Current Lamp Intensity Setting
u8	ubLampPowerOnLevel;				// Intensity level at power on
u8	ubLampSystemFailureLevel;		// Intensity level after system failure
u8	ubLampMinLevel;					// Minimum Lamp Intensity Setting
u8	ubLampMaxLevel;					// Maximum Lamp Intensity Setting
u8	ubLampFadeRate;					// Lamp fade rate F = 506/sqrt(2^X) steps/sec; X= 1..15
u8	ubLampFadeTime;					// Lamp fade time T = 1/2*sqrt(2^X) seconds ; X= 1..15
u16	uLampGroupFlags;				// List of groups this station belongs to
u8	ubLampScene[16];				// Intensity level for each scene
u8	ubLampStatus;					// lamp status
u8	ubLampVersion;					// const
u8	ubLampDeviceType;				// const
u8	ubPhysicalMinLevel;				// const



// Variables declared in plc.h
u16		plcMode = RX_MODE;		// Power Line Communications mode.  TX or RX
u16		plcModeSnap = RX_MODE;	// Power Line Communications mode.  TX or RX
u16		uRxMode = FIND_BITSYNC;	// PLC Receive Mode
u16		T1PIntCount;			// EV Timer1 Period Interrupt counts
//u16	T2PIntCount;			// EV Timer2 Period Interrupt counts
u16		ADCIntCount;			// ADC Interrupt counts
u16		uCmd_EchoAck;			// toggle this var between BER achnowledge packets

u16		txBitPending;			// Flag to indicate that a transmit bit is ready to be sent
u16		rxBitPending;			// Flag to indicate that a receive bit is ready to be decoded
u16		txBitNext;				// Value of next bit to be sent via PLC
u16		uRxMsgPending;		
volatile u16 uADCIntFlag = 0;

u16		uRxByteCount = 0;	// pointer to rxUserDataArray
u16 	uRxModeCount=0;		// number of ADC INT counts we have been in a given rxMode state

u16		rxUserDataArray[MAX_RX_MSG_LEN];	// byte-wide buffer for user data
u16		txUserDataArray[MAX_TX_MSG_LEN]; 	// byte-wide buffer for user data
u16		txDataArray[TX_ARRAY_LEN]; 			// word-wide byte-packed buffer for user transmit data, including headers, trailers, parity, and start/stop bits
u16 	rxDataArray[RX_ARRAY_LEN];			// word-wide byte-packed buffer for user receive data, including headers, trailers, parity, and start/stop bits

u32		ulPlcStats[PLC_STATS_LEN/2/2][2];	// Statistics for PLC communication
u32		ulBerStats[BER_STATS_LEN/2];		// Statistics for BER testing


// Variables declared in diag.h
u16	 uTraceTriggerLevel = (32767/10);	// (1/10th of 10V full-scale)
u16	 uTraceStatus;
u16	 uTraceNumVars;
u16* uppTraceVar[30];
asm("uppTraceVar_DIM1 .set 30");
u16	 uTracePointer;
u16	 uTraceStartCond;
u16	 uTraceStopCond;
u16	 uTraceDelayCount = 0;		// Num of blocks to collect after stop condition met (0=Stop immediately)
u16	 uTraceDelayCntr = 0;		// Counter for post-stop delay
u16	 uTraceSkipCount = 0; 		// Num of samples to skip between storing (0= No skip)
u16  uTraceSkipCntr = 0; 		// Counter for skipping samples
u16	 uTraceBufLen = TRACE_BUF_LEN;

#if (TRACE_BUF_LEN > 0)
	#ifdef __cplusplus			// "C++"
	#pragma DATA_SECTION("trc_buff");
	#else						// "C"
	#pragma DATA_SECTION(upTraceBuffer, "trc_buff");
	#endif
	
	u16 upTraceBuffer[TRACE_BUF_LEN];
	u16	uTraceIndex = 0;	
#endif


//---- gloval variables used in datadet.c -------------
pllControl		pll;		// global vars that control pll opperation




//==========================================================================================
// Function:		InitializeGlobals()
//
// Description: 	Assign initialization values to global variables.
//					This function gets called from main() as part of the setup process.
//
// Revision History:
// 05/13/02 EGO		Started function.
// 06/08/04 HEM		Clear the Power-Line Communication arrays.
// 11/1x/04	HEM		Removed unused vars left over from CAN project.
//==========================================================================================
void InitializeGlobals()
{
	u16		i;								// Generic loop index

	// Various global variable initializations...
	uVersion[0] = VERSION_NUMBER; 			// Unique Identifier for code build.
	uSampleNumber = 1;						// Servo period sample counter.
 	ulTimerIntCounter = 0;					// Reset the interrupt counter.

/*
	// Initialize the fixed location variables required by the tester.
	upBuildInfo[0] = (u16) &uVersion;			// Address of Version Number.
	upBuildInfo[1] = (u16) &uTraceNumVars;		// Address of TraceNumVars
	upBuildInfo[2] = (u16) &uppTraceVar[0];		// Address of uppTraceVar[]
	upBuildInfo[3] = (u16) PERIODS_PER_SEC;		// Servo sample period rate
*/

	uCommandActive = 0;						// No serial command yet.
	//uCalibActive = 0;						// Don't start calibrations yet.
	// = PERIODS_PER_SEC;


	// Clear Trace buffer to start.
	#if (TRACE_BUF_LEN > 0)
		for (i=0; i<TRACE_BUF_LEN; i++)
		{
			upTraceBuffer[i] = 0;
		}
	#endif
	
	// Clear PLC statisitics to start.
	for (i=0; i<16; i++)
	{
	
		ulPlcStats[i][RX_MODE] = 0;
		ulPlcStats[i][TX_MODE] = 0;
	}
	
	// Clear BER statisitics to start.
	for (i=0; i<BER_STATS_LEN; i++)
	{
	
		ulBerStats[i] = 0;
	}
	

	//---- clear rxUserDataArray ------------------
	memset(rxUserDataArray, 0, MAX_RX_MSG_LEN*sizeof(u16));

	return;
}


//==========================================================================================
// Revision History:
// 05/08/02	EGO		Started file.
// 03/13/03 HEM		Made uVersion into an array to hold info from all 3 nodes.
// 11/1x/04	HEM		Removed unused vars left over from CAN project.
//                  New var uADCIntFlag.
// 02/14/05 Hagen	Added uTraceIndex
// 02/17/05 Hagen	changed uBerStats to ulBerStats
//==========================================================================================



