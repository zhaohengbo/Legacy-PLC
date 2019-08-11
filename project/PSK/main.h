//==========================================================================================
// Filename:		main.h
//
// Description:		Main header file.
//					Contains compilation flags.
//					Contains typedefs for variable types
//					Contains structure definitions.
//					Contains function prototypes for all functions.
//					Contains global variables.
//					Contains global constants.
//					Contains macros.
//
// Copyright (C) 2002 Texas Instruments Incorporated
// Texas Instruments Proprietary Information
// Use subject to terms and conditions of TI Software License Agreement
//
// Revision History:	MOVED TO END OF FILE!
//==========================================================================================


#ifndef main_h							// Header file guard
#define main_h


//==========================================================================================
// Compilation Flags
//==========================================================================================
#define	False	(1==0)
#define True	(~False)


#ifdef  VCPP
	#define	MEX_COMPILE		True
	//#define	LOC_OFFICE		True
	#define	LOC_LAB		True
#else
	#define	DSP_COMPILE		True
	#define	LOC_LAB			True
#endif

#define MEX_VERBOSE			False

//---- Matlab library declarations --------------------------------
#ifdef MEX_COMPILE
	#ifdef LOC_OFFICE
		#include "C:\MATLAB6p1\extern\include\mex.h"
	#else
		#include "C:\NetMatlab12\extern\include\mex.h"
	#endif
#endif


//==========================================================================================
// Global Type Definitions
//==========================================================================================
typedef int 			s16;
typedef long 			s32;
typedef unsigned char 	u8;
typedef unsigned int 	u16;
typedef unsigned long 	u32;
typedef	s16				q16; 
typedef	s32				q32;  



//==========================================================================================
// System #include files <filename.h>
// Application #includes "filename.h"
//==========================================================================================
//#ifdef __cplusplus
//	#include <cstdlib>		// C++ standard library
//#endif

#include "DSP28_Device.h"	// DSP28 general file. device #includes, register definitions.
#include "prototypes.h"		// global prototype declarations.
#include "error.h"			// error codes.
//==========================================================================================
// Global Constants - General purpose constants
//==========================================================================================
#define SUCCESS			(0)			// Return code for success.

#define	NUM_CHANNELS	(4)			// Number of TEC controller channels in the system.
	asm("NUM_CHANNELS .set 4");

#define	COMMAND_PARMS   (32)        // Number of parameters in command.
	asm("COMMAND_PARMS .set 32");


#ifdef _Release
	#define TRACE_BUF_LEN	(0)			// No trace buffer, no diagnostics
	asm("TRACE_BUF_LEN .set 0H");
#else
	#define TRACE_BUF_LEN	(0x1A00)	// 4K trace buffer	--> increased to 6656 samples
	asm("TRACE_BUF_LEN .set 01A00H");
#endif

//==========================================================================================
// Global Constants - Related to hardware. Register bits, ...
//==========================================================================================
// BOARD_TYPE:  
	#define	EZDSP2407	2407
	#define	TEC_BOARD	2406
	#define EZDSP2812	2812
#define	BOARD_TYPE	EZDSP2812

#if BOARD_TYPE == EZDSP2812
	#define	DSP_TYPE	2812			// TMX320F2812

//	#define	DSP_FREQ	120000000L		// 30  MHz oscillator /2 * 8 = 120 MHz
//	#define MIPS		(120)
//	asm("MIPS	.set	120"); 
	#define	DSP_FREQ	150000000L		// 30  MHz oscillator /2 * 10 = 150 MHz
	#define MIPS		(150)
	asm("MIPS	.set	150"); 

#elif BOARD_TYPE == EZDSP2407
	#define	DSP_TYPE	2407			// TMS320LF2407A
	#define	DSP_FREQ	(14745600L*2)	// 14.7456 MHz oscillator / 2 * 4
	#define MIPS		(29)
	asm("MIPS	.set	29"); 
#else
	#define	DSP_TYPE	2406			// TMS320
	#define	DSP_FREQ	40000000L		// 10 MHz oscillator * 4		
	#define MIPS		(40)
	asm("MIPS	.set	40"); 
#endif

// Processor Definition.
#define	DSP28	2810
#define	ARM		470
#define	DSP24	2407


//// DAC_SIZE = Output size of PWM DAC hardware = 9 bits
//#define DAC_SIZE		9      
  
//// DAC_SIZE = Output size of PWM DAC hardware = 10 bits
//#define	DAC_SIZE		10

// DAC_SIZE = Output size of PWM DAC hardware = 15 bits
#define	DAC_SIZE		15

// Q_CURR = Scaling of qVoltsOut and other currents (q2.14)
#define	Q_CURR			14                 


#define TASK_SWITCH_RX True
//#define TASK_SWITCH_RX (~True)

// These two values must match each other
   asm(" .def	FIR_LEN");   
   asm("FIR_LEN	.equ	25");
#define	FIR_LEN			25				// Length of FIR filter
//#define	FIR_OUT_LEN		(FIR_LEN*12)	// Size of FIR output buffers.  

										// May not need to be so long.

#define	FIR_OUT_LEN		FIR_LEN		// Size of FIR output buffers. 
//#define	FIR_OUT_LEN		(FIR_LEN*2)		// Size of FIR output buffers.  
#define CORR_OUT_LEN	(FIR_LEN+1)	// MUST be longer than FIR_LEN
//#define CORR_OUT_LEN	(FIR_LEN*2)	// MUST be longer than FIR_LEN
//#define CORR_OUT_LEN	(FIR_LEN*48)	// MUST be much longer than FIR_LEN
//??? FIR_LEN*MAX_TX_MSG_LEN


//==========================================================================================
// Timer constants.
// The timer interrupt is generated by the same clock that controls the PWM DACs, so it must
// be divided down to get the servo period.
//
// With a 40 MHz DSP clock and a 9-bit PWM DAC resolution, the PWM DAC runs at 78.125 kHz.
// The desired servo sample rate is 100 Hz.  At that rate the timer interrupt occurs 781
//  times per servo period.
//==========================================================================================
#define PERIOD_SRC		0					// Use DSP Timer for periodic timing only
//#define PERIOD_SRC		1				// Use Timer 1 for PWM and periodic timing
//#define PERIOD_SRC		2				// Use Timer 2 for periodic timing only

//#define PERIODS_PER_SEC		100
//#define PERIODS_PER_SEC		500
//#define	PERIODS_PER_SEC		825
//#define	PERIODS_PER_SEC		2500 
#define	PERIODS_PER_SEC		1000 

#if (PERIOD_SRC == 1)
	#define TINTS_PER_SEC	(DSP_FREQ>>DAC_SIZE) 	// Using Timer 1 for periodic timer and PWM
#else
//	#define	TINTS_PER_SEC	1000 					// Using Timer 2 for 1 kHz periodic timer
//	#define	TINTS_PER_SEC	2000 					// Using Timer 2 for 2 kHz periodic timer
	#define	TINTS_PER_SEC	(PERIODS_PER_SEC*2)		// Using Timer 2 for 2 interrupts per period
#endif

#define	TINTS_PER_PERIOD	(TINTS_PER_SEC / PERIODS_PER_SEC)


//==========================================================================================
// GPIO definitions.
//==========================================================================================
#define	GPIO_FPGA_PRESENT	(1)
#define	GPIO_FPGA_ABSENT	(0)

#define	ON				1
#define OFF				0

#define AUTO_MODE		1
#define MANUAL_MODE		0

//==========================================================================================
// Command numbers
//==========================================================================================
#define	CMD_READ_MEMORY					(0x0001)
#define	CMD_WRITE_MEMORY				(0x0002)
#define	CMD_READ_STATS					(0x0003)
#define	CMD_CONTROL						(0x0004)
#define	CMD_CONSTANT_OUTPUT				(0x0005)
#define CMD_LASER_CONTROL				(0x0006)
#define	CMD_LAMP_DIRECT					(0x000A)
#define	CMD_LAMP						(0x000B)
#define CMD_PLC_COMMAND					(0x000C)
#define	CMD_CONFIG_CORRUPTER			(0x000D)
#define	CMD_CONFIG_FLOODER				(0x000E)		
#define	CMD_LOCAL_ADDRESS				(0x000F)		

#define	CMD_DIAG_TRACE_CONFIG			(0x0010)

#define	CMD_ECHO_SET					(0x0020)
#define	CMD_ECHO_CMD					(0x0021)
#define	CMD_ECHO_ACK					(0x0022)

//==========================================================================================
// Command parm number descriptions by command
//==========================================================================================
// Common
#define NUMBER							(0)
#define FLAG							(1)
#define FLAG2							(2)	// Only for commands which require it

// Read Memory
#define RM_COUNT						(2)
#define RM_ADDRESS						(3)
// Write Memory
#define	WM_COUNT						(2)
#define WM_ADDRESS						(3)
#define WM_DATA							(4)
// Calibrate
#define CAL_CHANNEL						(2)
// Control Temperature
#define CTRL_TEMP						(2)
// Constant Output
#define OUT_TEC_VALUE					(2)
#define OUT_LASER_VALUE					(6)
#define OUT_AUX_VALUE					(10)
// Diag Trace
#define TRACE_VARIABLE					(3)
#define LIST_MODIFIER					(3)
// Echo Flags
#define BER_READ						(1)
#define BER_RESET						(2)


//==========================================================================================
// Bit masks for Flag Registers
//==========================================================================================
// Read Memory
#define RM_MODE							(0x0002)
#define RM_MODE_ADDR					(0x0000)
#define RM_MODE_BLOCK					(0x0002)
// Write Memory
#define WM_MODE							(0x0002)
#define WM_MODE_ADDR					(0x0000)
#define WM_MODE_BLOCK					(0x0002)
// Calibrate
//#define CAL_XXX						(0x0001)
// Control
#define CTRL_MODE						(0x0001)
#define CTRL_MODE_REL					(0x0000)
#define CTRL_MODE_ABS					(0x0001)
#define CTRL_SAT						(0x0002)
#define CTRL_0INT						(0x0004)
#define CTRL_APPLY						(0x0008)
// Constant Output
#define OUT_TEC							(0x000F)
#define OUT_TEC0						(0x0001)
#define OUT_TEC1						(0x0002)
#define OUT_TEC2						(0x0004)
#define OUT_TEC3						(0x0008)
#define OUT_LASER						(0x00F0)
#define OUT_LASER0						(0x0010)
#define OUT_LASER1						(0x0020)
#define OUT_LASER2						(0x0040)
#define OUT_LASER3						(0x0080)
#define OUT_AUX							(0x0300)
#define OUT_AUX0						(0x0100)
#define OUT_AUX1						(0x0200)
// Diag Trace
#define TRACE_EN						(0x0001)
#define TRACE_RESET						(0x0002)
#define TRACE_HALT_TRIG					(0x0004)
#define TRACE_SPARE1					(0x0008)
#define TRACE_SPARE2					(0x0010)
#define TRACE_SPARE3					(0x0020)
#define TRACE_VAR						(0x07C0)
#define TRACE_VAR0						(0x0040)
#define TRACE_LIST						(0xF800)
#define TRACE_LIST0						(0x0800)
#define TRACE_LIST_USER					(0x0000)
#define TRACE_LIST_1   					(0x0800)
#define TRACE_LIST_2   					(0x1000)
#define TRACE_LIST_3   					(0x1800)
#define TRACE_LIST_4   					(0x2000)
#define TRACE_LIST_5   					(0x2800)
#define TRACE_LIST_6   					(0x3000)
#define TRACE_LIST_7					(0x3800)
#define TRACE_LIST_8					(0x4000)
// Room for 9-31...
#define TRACE_LIST_31					(0xF800)
// Diag Trace Flag2
// See TSTART_?? and TSTOP_?? definitions for defined start and stop conditions.
#define TRACE2_START					(0x00FF)
#define TRACE2_START0					(0x0001)
#define TRACE2_STOP						(0xFF00)
#define TRACE2_STOP0					(0x0100)
// Diag TFA
#define TFA_AXIS						(0x0001)
#define TFA_AXIS_X						(0x0000)
#define TFA_AXIS_Y						(0x0001)
#define TFA_MIRROR						(0x0006)
#define TFA_MIRROR0						(0x0002)
#define TFA_RESEED						(0x0008)



// CAN Bus Message Mailboxes
#define		BUS_SPY_MBX			(1)
#define 	MOTOR_CMD_MBX		(2)
#define		MOTOR_STAT_MBX		(3)
#define		FAN_CMD_MBX			(4)
#define		FAN_STAT_MBX		(5)
#define		THERM_DATA_MBX		(7)
#define		HEATER_CMD_MBX		(8)
#define		HEATER_STAT_MBX		(9)
#define		LAMP_CMD_MBX		(10)
#define		LAMP_STAT_MBX		(11)
#define		PHOTO_DATA_MBX		(13)
#define		FLOOD_CMD_MBX		(14)
#define		FLOOD_DATA_MBX		(15)


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
#define HighWord(x)	( (s16)((x)>>16) )
#define LowWord(x)	( (s16) (x) )   
#define HighByte(x)	( (u8)(((x)&0xFF00)>>8) )
#define LowByte(x)	( (u8) ((x)&0x00FF) )   

#define 	SATMACROS	(~(True))	//~True ==> Use assembly functions for speed
//#define 	SATMACROS	(True)		// True ==> Use C macros for maximum portability

	#define Max(A,B) ((A) > (B) ? (A):(B)) 
	#define Min(A,B) ((A) < (B) ? (A):(B))	
//	#define USaturate(x,LoLim,HiLim) Max((LoLim), Min((HiLim), (x) ) )
//	#define Saturate(x,LoLim,HiLim) Max((LoLim), Min((HiLim), (x) ) )
	#define Saturate(x,LoLim,HiLim) ((x) > (HiLim) ? (HiLim) : ((x) < (LoLim) ? (LoLim) : (x)))

#if (SATMACROS == True)
	#define Sat16(x) Saturate((x), -32767, 32767)
	#define LSaturate(x,LoLim,HiLim) Max((LoLim), Min((HiLim), (x) ) )
#else
#ifdef __cplusplus
extern "C"	{
#endif
	extern q16	Sat16(q32 qlX);			// Saturate value to +/-32767
//	extern q16	Saturate(q32 qlVal, q32 qlLoLim, q32 qlHiLim);
	#define LSaturate(x,LoLim,HiLim) Saturate(x,LoLim,HiLim)
#ifdef __cplusplus
}
#endif
#endif


#define IsEven(x) ((((x)>>1)<<1) == (x) ? 1:0)
#define IsOdd(x)  ((((x)>>1)<<1) != (x) ? 1:0)

// Bit Macros:
#define SetBits(var, mask) 				((var) |= (mask))
#define ClearBits(var, mask) 			((var) &= (~(mask)))
#define AssignBits(var, mask, val) 		((var) = (((var)&(~(mask)))|((val)&(mask))))
#define GetBits(var, mask)				((var) & (mask))
#define TestBits(var, mask, val)		((((var) & (mask)) == (val)) ? 1 : 0)
#define TestBit(var, val)				((var)&(val) ? 1:0)

// defines related to setting/clearing XF external flag pin 
#define SetXF() 	asm(" SETC	XF	")
#define ClearXF()	asm(" CLRC	XF	")

// Do a single nop instruction 
#define NOP	asm("	NOP")

// Macros for generating controlled short delays      
// The value of n must be between 2 and 257.
#define RPTNOP(n)	asm("	RPT #((" #n ")-2)");\
					asm(" ||NOP")

// Delay1us will delay for 1 us.  Unfortunately, it will lock out interrupts during the
// delay, so be very careful when using this in time-critical routines.
#define	Delay1us()	RPTNOP(MIPS)


//==========================================================================================
// GPIO definitions used to make subsequent bit twiddling easy.
//==========================================================================================
//#define	STATUS_LED0		GpioDataRegs.GPADAT.bit.GPIOA8
//#define	STATUS_LED1		GpioDataRegs.GPADAT.bit.GPIOA9
//#define	STATUS_LED2		GpioDataRegs.GPADAT.bit.GPIOA10

#define	PLC_RX_GOOD_LED	GpioDataRegs.GPADAT.bit.GPIOA8		// Green LED - Good PLC message received 
#define	PLC_RX_BUSY_LED	GpioDataRegs.GPADAT.bit.GPIOA9		// Yellow LED - PLC message receive in progress
#define	PLC_TX_LED		GpioDataRegs.GPADAT.bit.GPIOA10		// Red LED - PLC transmit message in progress

//#define	OUTPUT_A11		GpioDataRegs.GPADAT.bit.GPIOA11
//#define	CAN_TERM_EN		GpioDataRegs.GPADAT.bit.GPIOA11
#define	TX_BIAS_EN		GpioDataRegs.GPADAT.bit.GPIOA11

//TEC #define	TEMP0_GOOD		GpioDataRegs.GPADAT.bit.GPIOA8				//TEMP!!! Get rid of these
//TEC #define	TEMP1_GOOD		GpioDataRegs.GPADAT.bit.GPIOA9				//TEMP!!! Get rid of these
//TEC #define	TEMP2_GOOD		GpioDataRegs.GPADAT.bit.GPIOA10				//TEMP!!! Get rid of these
//TEC #define	TEMP3_GOOD		GpioDataRegs.GPADAT.bit.GPIOA11				//TEMP!!! Get rid of these
//TEC #define	ERROR0			GpioDataRegs.GPADAT.bit.GPIOA12				//TEMP!!! Get rid of these
//TEC #define	ERROR1			GpioDataRegs.GPADAT.bit.GPIOA13				//TEMP!!! Get rid of these
//TEC #define	ERROR2			GpioDataRegs.GPADAT.bit.GPIOA14				//TEMP!!! Get rid of these
//TEC #define	ERROR3			GpioDataRegs.GPADAT.bit.GPIOA15				//TEMP!!! Get rid of these


#define	LAMP_ON_LED		GpioDataRegs.GPADAT.bit.GPIOA12
#define	LAMP_MODE_LED	GpioDataRegs.GPADAT.bit.GPIOA13
#define	FAN_ON_LED		GpioDataRegs.GPADAT.bit.GPIOA14
#define	FAN_MODE_LED	GpioDataRegs.GPADAT.bit.GPIOA15

#define	CORRUPTER_MODE0	GpioDataRegs.GPBDAT.bit.GPIOB4
#define	CORRUPTER_MODE1	GpioDataRegs.GPBDAT.bit.GPIOB5
#define	CORRUPTER_MODE2	GpioDataRegs.GPBDAT.bit.GPIOB6
#define	CORRUPTER_EN	GpioDataRegs.GPBDAT.bit.GPIOB7

// These jumpers are wired with pull-ups, so invert polarity here.
#define INPUT_B8		(GpioDataRegs.GPBDAT.bit.GPIOB8^1)
#define INPUT_B9		(GpioDataRegs.GPBDAT.bit.GPIOB9^1)
#define INPUT_B10		(GpioDataRegs.GPBDAT.bit.GPIOB10^1)
#define INPUT_B11		(GpioDataRegs.GPBDAT.bit.GPIOB11^1)

#define	INPUT_B12		GpioDataRegs.GPBDAT.bit.GPIOB12

// These switches are wired with pull-ups, so invert polarity here.
#define	FAN_MODE_SW		(GpioDataRegs.GPBDAT.bit.GPIOB13^1)
#define	FAN_ON_SW		(GpioDataRegs.GPBDAT.bit.GPIOB14^1)
#define	FAN_OFF_SW		(GpioDataRegs.GPBDAT.bit.GPIOB15^1)

#define UART_RTS		GpioDataRegs.GPFDAT.bit.GPIOF0
#define UART_CTS		GpioDataRegs.GPFDAT.bit.GPIOF1

#define CANTX_LED		GpioDataRegs.GPFDAT.bit.GPIOF2
#define CANRX_LED		GpioDataRegs.GPFDAT.bit.GPIOF3

// Macro for turning LEDs on and off
#define	SetLED(led,state)	(led)=(state)
		
								   

typedef struct 
{
	s16				eta;		// PLL persitant variables	
	u16				phase;			
	u16				bitPhase;        
	s16				cycle;
	u16				firCnt;			
	
	s16				phaseHold;
	s16				intPhase;
	s16				proPhase;
	
	s16				phcos;
	s16				phsin;
	u16				polarity;
	u16				hystCnt;

	s16				sigPower;
	s16				sGainIndex;
	s16				agcHold;
}	pllControl;



#define ADCINT_COUNT_MAX  	7	// rollover count for global variable ADCIntCount
								   						   						   						   
//==========================================================================================
// Global Variables declarations.  (matching definitions found in "vardefs.h")
//==========================================================================================
extern u16	uVersion[3];					// Unique Identifier for code build.
extern u16	uMyAddress;						// My address
extern u16	uDestAddress;					// Destination address for BER packets
extern u16	uSampleNumber;					// 
extern u16  uStartFlag;						// Stagger-start control flag

extern volatile u32	ulTimerIntCounter;		// Counter for timer interrupts.
											// Updated by periodic timer ISR, so must be volatile

extern u16	uFloodInterval;					// Number of TINT interrupts between fake PLC messages

extern u16	uFadeInterval;					// Number of TINT interrupts between lamp fader steps
extern u8	ubLampDTR;						// Lamp Controller Data Transfer Register
extern u8	ubLampIntensityTarget;			// Lamp Intensity Target Setting
extern u8	ubLampIntensity;				// Current Lamp Intensity Setting
extern u8	ubLampPowerOnLevel;				// Intensity level at power on
extern u8	ubLampSystemFailureLevel;		// Intensity level after system failure
extern u8	ubLampMinLevel;					// Minimum Lamp Intensity Setting
extern u8	ubLampMaxLevel;					// Maximum Lamp Intensity Setting
extern u8	ubLampFadeRate;					// Lamp fade rate F = 506/sqrt(2^X) steps/sec; X= 1..15
extern u8	ubLampFadeTime;					// Lamp fade time T = 1/2*sqrt(2^X) seconds ; X= 1..15
extern u16	uLampGroupFlags;				// List of groups this station belongs to
extern u8	ubLampScene[16];				// Intensity level for each scene
extern u8	ubLampStatus;					// lamp status
extern u8	ubLampVersion;					// const
extern u8	ubLampDeviceType;				// const
extern u8	ubPhysicalMinLevel;				// const
		
extern u16	upCommand[COMMAND_PARMS];		// Local copy of last command.
extern u16	upSerialCommand[COMMAND_PARMS];	// Buffer to hold command from serial port.
extern u16	uCommandPending;				// Signals a full command has been received.
extern u16	uCommandActive;					// Indicates a command is being run.
//extern u16	uCalibActive;					// Indicates calibrations are being run.
//extern u16	uSampleRate;					// SampleRate variable for tester use.

// Integrator gets reset in command.c as well as used in control.c
extern q32 	qlPID_i[NUM_CHANNELS];

extern u16 	uHeaterCmdState;				// Commanded Heater State
extern u16 	uHeaterState;					// Heater State as reported by Fan Node
 
extern u16	uFanCmdState;					// Commanded Fan State
extern u16	uFanState;						// Fan State as reported by Fan Node

extern q16	qTherm1Data;					// Thermal sensor 1 data
extern q16	qTherm2Data;					// Thermal sensor 2 data
 
extern u16	uLampCmdState;					// Commanded Lamp State
extern u16	uLampState;						// Lamp State as reported by SysMon Node
 
extern u16	uMotorCmdState;					// Commanded Motor State
extern u16	uMotorState;					// Motor State as reported by Motor Node	
extern q16	qMotorCmdPos;					// Commanded Motor Position
extern q16	qMotorPos;						// Motor Position as reported by Motor Node	
extern q16	qMotorCmdSpeed;					// Commanded Motor Speed	
extern q16	qMotorSpeed;					// Motor Speed as reported by Motor Node	

extern u16  uFloodMissingCntr;				// Number of Missing Flood packets
extern u16  uPhotoData;						// Reading from photo sensor 

extern u16	uTxMsgPending;					// Flag:Transmit message waiting to be sent
extern u16	uRxMsgPending;					// Flag: Received message waiting to be processed
extern volatile u16	uADCIntFlag;			// Flag: ADC interrupt has recently run
extern u16	ADCIntCount;					// ADC Interrupt counts
extern u16	uCmd_EchoAck;					// toggle this var between BER achnowledge packets

enum {RX_MODE, TX_MODE};
extern u16	plcMode;						// Power Line Communications mode.  TX or RX
extern u16	plcModeSnap;					// Snapshot of Power Line Communications mode.  TX or RX.

#define USE_CRC				True			// calculate, transmit and compate at receive a 16 bit CRC
#define	RECEIVE_OWN_XMIT	True			// Enable this line to allow us to receive our own transmitted signal		

//enum {FIND_BITSYNC1, FIND_BITSYNC2, FIND_ZEROCROSS, FIND_WORDSYNC, FIND_DATA};
enum {FIND_BITSYNC, FIND_WORDSYNC, FIND_DATA, FIND_EOP, EOP_HOLD_OFF};
extern u16	uRxMode;						// PLC Receive Mode

extern u16		uRxByteCount;				// pointer to rxUserDataArray

#define	MAX_RX_MSG_LEN	36					// Maximum receive message length (bytes)
extern u16	rxUserDataArray[MAX_RX_MSG_LEN];// byte-wide buffer for user data

#define	MAX_TX_MSG_LEN	32					// Maximum transmit message length (bytes)
//#define	MAX_TX_MSG_LEN	16					// Maximum transmit message length (bytes)
extern u16	txUserDataArray[MAX_TX_MSG_LEN];// byte-wide buffer for user data

#define	HEADER_LEN		(24+11)
#define	TRAILER_LEN		(11*2)
#define	CRCF_LEN			(11*2)
#define	TX_ARRAY_LEN	((HEADER_LEN + MAX_TX_MSG_LEN*11 + CRCF_LEN + TRAILER_LEN + 15)/16)	
extern u16	txDataArray[TX_ARRAY_LEN]; 			// word-wide byte-packed buffer for user transmit data, including headers, trailers, parity, and start/stop bits

#define	RX_ARRAY_LEN	TX_ARRAY_LEN			// Make rx array same length as tx array.  Really could be shorter, since much of header is not stored
extern u16	rxDataArray[RX_ARRAY_LEN];			// word-wide byte-packed buffer for user receive data, including headers, trailers, parity, and start/stop bits

//---- define constants and buffer for debug counters -------------------
#define PLC_STATS_LEN	(16*2*2)
enum {	
	TX_CNT, 				// 0		
	TX_COLLISION, 			// 1
	RX_CNT, 				// 2
	RX_GOOD, 				// 3
 	RX_PREDET_COUNT, 		// 4
 	RX_SYNCDET_COUNT,		// 5
	RX_EOP_COUNT,			// 6

	RX_ERR_WORDSYNC_TO,		// 7
 	RX_EOP_TIMEOUT,			// 8
 	RX_MSGLEN_ERROR,		// 9
	RX_ERR_CRC, 			// 10
	RX_ERR_PARITY 			// 11
	};

extern	u32	ulPlcStats[PLC_STATS_LEN/2/2][2];		// Statistics for PLC communication

//---- define constants and variables for BER testing ---------------
#define	BER_STATS_LEN		(4*2)
enum { 
	BER_TX_COUNT, 			// 0
	BER_ECHO_COUNT,			// 1
	BER_ACK_COUNT,			// 2
	BER_NZERO_COUNT			// 3
	};
extern	u32	ulBerStats[BER_STATS_LEN/2];			// Statistics for BER testing


// Trace buffer global variable declarations and values.
#if (TRACE_BUF_LEN > 0)
	extern u16	upTraceBuffer[TRACE_BUF_LEN];	// This is the trace buffer.
	extern u16	uTraceIndex;					// Trace buffer index
	extern u16	uTraceStatus;					// Trace buffer status register
	
	#define		TS_ENABLE 		(0x0001)		// TraceStatus bit 0 = Enable.
	#define		TS_TRIGGERED	(0x0002)		// TraceStatus bit 1 = Triggered.
	#define		TS_HALT_TRIG	(0x0004)		// TraceStatus bit 2 = Disable on trigger.
	extern u16	uTraceStartCond;				// Trace buffer start (trigger) condition
	#define		TSTART_IMMED	(0x0000)		// Start condition: trigger immediately
	#define		TSTART_CMD		(0x0001)		// Start condition: start of any command 
	#define		TSTART_MOTION	(0x0002)		// Start condition: start of seek, output, or cal.
	extern u16	uTraceStopCond;					// Trace buffer stop condition
	#define	   	TSTOP_FULL		(0x0000)		// Stop condition: buffer full.
	#define	   	TSTOP_CMD_CMPL	(0x0001)		// Stop condition: command complete
	#define		TSTOP_EXT_TRIG	(0x0003)		// Stop condition: External Trigger
	#define	   	TSTOP_NEVER		(0x00FF)		// Stop condition: none (run forever)
#endif
//==========================================================================================
#endif									// End of header guard: #ifndef main_h


/*==========================================================================================
	Revision History:

	05/08/01	EGO		Started file.
	03/20/03	HEM		Stripped out unused stuff from TEC project.
	11/17/04 	HEM		Removed unused vars left over from CAN project.
                		New var uADCIntFlag.
						New compile-time option TASK_SWITCH_RX.
	24Feb05		Hagen	removed RX_ERR_ZEROCROSS from enum list for plcStats
	28Feb05		Hagen	Put #ifdef _release around upTraceBuffer declaration
	07Mar05		Hagen	changed TRUE to True and FALSE to False
==========================================================================================*/





