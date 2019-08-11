//==========================================================================================
// Filename:		timer.c
//
// Description:		Functions for the on-board timers and delay generation.
//
// Copyright (C) 2000 - 2002 Texas Instruments Incorporated
// Texas Instruments Proprietary Information
// Use subject to terms and conditions of TI Software License Agreement
//
// Revision History:
// 05/15/02	HEM		Started file. 
//==========================================================================================

#include "main.h"


//==========================================================================================
// Local function prototypes
//==========================================================================================
void ConfigureDog(void);
void WakeDog(void);
void SleepDog(void);
void FeedDog(void);


//==========================================================================================
// External Variables
//==========================================================================================


//==========================================================================================
// Local constants
//==========================================================================================


//==========================================================================================
// Function:		DelayNus(N)
//
// Description: 	This function delays N microseconds.  It locks out interrupts for 1 us
//					at a time, so use cautiously in time-critical routines.
//
// Revision History:
//==========================================================================================
void DelayNus(u16 uN)
{
	u16		i;	// loop counter
	for (i=1; i<uN; i++)
	{
		RPTNOP(MIPS-10);	// Delay 1 us per loop (locks out interrupts)
	} 
}      

  
//==========================================================================================
// Function:		DelayNms(N)
//
// Description: 	This function delays N milliseconds.  It locks out interrupts for 1 us
//					at a time, so use cautiously in time-critical routines.
//
// Revision History:
// 02/01/02	HEM		New function.
//==========================================================================================
void DelayNms(u16 uN)
{
	u16		i;	// loop counter
	for (i=1; i<uN; i++)
	{
		DelayNus(1000);		// Delay 1 ms per loop 
	} 
}      


//==========================================================================================
// Function:		ConfigureDog()
//
// Description: 	This function disables the WatchDog timer and sets its prescale ratio.
//
// Revision History:
// 05/15/02 HEM		New function.
// 06/17/02 EGO		Use C28 structures for registers.	
//==========================================================================================
void ConfigureDog(void)
{
	//The watchdog timer uses the DSP clock divided by 512 as its input, so a 40 MHz DSP 
	// clock will produce a 78.125 kHz WDCLK.  The watchdog timer's output rollover rate 
	// is set by the lower three bits in the WDCR register.  The overflow frequencies and
	// periods are shown for a 40 MHz DSP clock.

	// WatchDog PreScale	Div Rate	Overflow Freq	Overflow Period
	#define	WDPS_1X		1	// 	1x		305.2 kHz		  3.28 ms
	#define	WDPS_2X		2	// 	2x		152.6 kHz 		  6.6 ms
	#define	WDPS_4X		3	// 	4x		 76.3 kHz 		 13.1 ms
	#define	WDPS_8X		4	// 	8x		 38.1 kHz 		 26.2 ms
	#define	WDPS_16X	5	// 	16x		 19.1 kHz 		 52.4 ms
	#define	WDPS_32X	6	// 	32x 	  9.5 kHz 		104.9 ms
	#define	WDPS_64X	7	// 	64x		  4.8 kHz 		209.7 ms
	
	#define WDPS		WDPS_64X	// Use slowest watchdog rate

	SysCtrlRegs.WDCR = (SysCtrlRegs.WDCR & (~0x007F)) | 0x0068 | WDPS; 	// Disable WatchDog
}


//==========================================================================================
// Function:		WakeDog()
//
// Description: 	This function ensables the WatchDog timer.
//
// Revision History:
// 05/15/02 HEM		New function.
// 06/17/02 EGO		Use C28 structures for registers.	
//==========================================================================================
void WakeDog(void)
{
	SysCtrlRegs.WDCR = (SysCtrlRegs.WDCR & (~0x0078)) | 0x0028; 	// Enable WatchDog
}


//==========================================================================================
// Function:		SleepDog()
//
// Description: 	This function disables the WatchDog timer.
//
// Revision History:
// 05/15/02 HEM		New function.
// 06/17/02 EGO		Use C28 structures for registers.	
//==========================================================================================
void SleepDog(void)
{
	SysCtrlRegs.WDCR = (SysCtrlRegs.WDCR & (~0x0078)) | 0x0068; 	// Disable WatchDog
}


//==========================================================================================
// Function:		FeedDog()
//
// Description: 	This function resets the WatchDog timer.
//
// Revision History:
// 05/15/02 HEM		New function.
// 06/17/02 EGO		Use C28 structures for registers.	
// 					Added EALLOW/EDIS.
// 06/17/02 HEM		Cleared up warning message by removing "static inline" in declaration.
//==========================================================================================
void FeedDog(void)
{
	EALLOW;
	SysCtrlRegs.WDKEY = 0x5555;		
	SysCtrlRegs.WDKEY = 0xAAAA;
	EDIS;
}


//==========================================================================================
// Function:		ConfigureGPTimers()
//
// Description: 	This function configures the GP timer periods.
//
// Revision History:
// 05/25/04 HEM		Start over from scratch using newer bit definitions.
//==========================================================================================
void ConfigureGPTimers(void)
{
	IFR = IFR;				// Read all interrupt flags and clear them

	SleepDog();				// Disable the watch-dog timer
	FeedDog();				// Reset the watch-dog timer
	
	// ----- Disable all EV Timers while configuring them -----
	EvaRegs.T1CON.bit.TENABLE = 0;	//Disable EV Timer1   
	EvaRegs.T2CON.bit.TENABLE = 0;	//Disable EV Timer2
	EvbRegs.T3CON.bit.TENABLE = 0;	//Disable EV Timer3
	EvbRegs.T4CON.bit.TENABLE = 0;	//Disable EV Timer4


	// ----- Set up EV interrupts -----
	EvaRegs.EVAIMRA.all = 0;			// Disable interrupts from EVA module
	EvaRegs.EVAIMRB.all = 0;			
	EvaRegs.EVAIMRA.bit.T1PINT = 1;		// Enable interrupt from Timer1	
//?	EvaRegs.EVAIMRB.bit.T2PINT = 1;		// Enable interrupt from Timer2
	
	EvaRegs.EVAIFRA = EvaRegs.EVAIFRA;	// Clear the EVA interrupts
	EvaRegs.EVAIFRB = EvaRegs.EVAIFRB;	// Clear the EVA interrupts


	EvbRegs.EVBIMRA.all = 0;			// Disable interrupts from EVB module
	EvbRegs.EVBIMRB.all = 0;			// Disable interrupts from EVB module

	EvbRegs.EVBIFRA = EvbRegs.EVBIFRA;	// Clear the EVB interrupts
	EvbRegs.EVBIFRB = EvbRegs.EVBIFRB;	// Clear the EVB interrupts


	// ----- Set up External Control -----
	EvaRegs.EXTCON.all = 0;				// configure EVA ext control reg to let COMCONA control output enable	
	EvaRegs.EXTCON.bit.EVSOCE = 1;		// DEBUG: Put out a 32x active-low pulse on the EVSOC pin on ADC SOC event
										// Does not affect EVTOADC signal routed to the ADC module as optional SOC trigger.
//	EvaRegs.EXTCON.bit.INDCOE = 1;		//??? Allow independent control of the timer compare hardware
//										//??? Lets the T2PWM_T2CMP pin pulse when the T2 compare event occurs
//										//??? May have other side effects as well.
	



	// ----- Set up GPTCON -----		

////	#define	GPTCON_INIT		0x60C5			// T1PWM&T2PWM active low, T4 
//	#define	GPTCON_INIT		0x6045			// T1PWM&T2PWM active low
//	EvaRegs.GPTCONA.all = GPTCON_INIT;	// Configure GP Timer A
	
	EvaRegs.GPTCONA.all = 0;			// Clear GP Timer A
	
	
	#define TOADC_NONE		0	// 00 No event starts ADC
	#define TOADC_UNDERFLOW	1	// 01 Setting of underflow interrupt flag starts ADC
	#define TOADC_PERIOD	2	// 10 Setting of period interrupt flag starts ADC
	#define TOADC_COMPARE	3	// 11 Setting of compare interrupt flag starts ADC
	
	EvaRegs.GPTCONA.bit.T2CMPOE = 1;			// 5	Timer2 compare output 
	EvaRegs.GPTCONA.bit.T2TOADC = TOADC_PERIOD;	// 10:9	Trigger ADC SOC with Timer2 Event	
		
//   Uint16 T1PIN:2;         // 1:0   Polarity of GP timer 1 compare
//   Uint16 T2PIN:2;         // 3:2   Polarity of GP timer 2 compare
//   Uint16 T1CMPOE:1;       // 4 	Timer1 compare output
//   Uint16 T2CMPOE:1;       // 5     Timer2 compare output 
//   Uint16 TCOMPOE:1;       // 6     Compare output enable
//   Uint16 T1TOADC:2;       // 8:7   Start ADC with timer 1 event
//   Uint16 T2TOADC:2;       // 10:9  Start ADC with timer 2 event




	// ----- Set up T1PR -----
	#define	TX_FREQ		(2500000/19) 						// Echelon transmit frequency = 131.57894 kHz
	#define TX_TPR		(((DSP_FREQ/TX_FREQ) + 1)/2)		// Transmit setting
	#define	RX_TX_DRIFT	(+0)								// Intentionally misalign the transmit and receive frequency so they drift in and out of phase quickly.  Prevent long stretches where they are out of phase and vulnerable to noise.
	#define	RX_TPR		(RX_TX_DRIFT+(DSP_FREQ/7*8/TX_FREQ))	// Receive setting. sample rx 21 times for every 24 tx periods.		
	
	EvaRegs.T1PR = TX_TPR; 			// Set timer 1 period register to transmit frequency
	EvaRegs.T1CNT = 0;				// Load the timer1 starting count

 	EvaRegs.T2PR = RX_TPR-1;		// Set timer 2 period register to receive frequency	
	
 	EvbRegs.T3PR = 1U<<DAC_SIZE;		// Set timer 3 period register

	// ----- Set up ACTRA ----- 
	#define	FORCE_LOW	0	// 00 Forced low
	#define	ACTIVE_LOW	1	// 01 Active low
	#define	ACTIVE_HIGH	2	// 10 Active high
	#define	FORCE_HIGH	3	// 11 Forced high
	
	EvaRegs.ACTRA.bit.CMP1ACT = ACTIVE_HIGH;	// 1:0    Action on compare output pin 1 CMP1	
	EvaRegs.ACTRA.bit.CMP2ACT = FORCE_LOW;		// 3:2    Action on compare output pin 2 CMP1
	EvaRegs.ACTRA.bit.CMP3ACT = ACTIVE_HIGH;	// 5:4    Action on compare output pin 3 CMP2
	EvaRegs.ACTRA.bit.CMP4ACT = FORCE_LOW;		// 7:6    Action on compare output pin 4 CMP2
	EvaRegs.ACTRA.bit.CMP5ACT = FORCE_LOW;		// 9:8    Action on compare output pin 5 CMP3
	EvaRegs.ACTRA.bit.CMP6ACT = FORCE_LOW;		// 11:10  Action on compare output pin 6 CMP3	
	EvaRegs.ACTRA.bit.D = 0;					// 14:12  Basic state vector bits	
	EvaRegs.ACTRA.bit.SVRDIR = 0;				// 15     Space vecor PWM rotation dir

	// ----- Set up ACTRB ----- 
	EvbRegs.ACTRB.bit.CMP7ACT = FORCE_LOW;		// 1:0    Action on compare output pin 7 CMP4	
	EvbRegs.ACTRB.bit.CMP8ACT = FORCE_LOW;		// 3:2    Action on compare output pin 8 CMP4
	EvbRegs.ACTRB.bit.CMP9ACT = ACTIVE_LOW;		// 5:4    Action on compare output pin 9 CMP5
	EvbRegs.ACTRB.bit.CMP10ACT = FORCE_LOW;		// 7:6    Action on compare output pin 10 CMP5
	EvbRegs.ACTRB.bit.CMP11ACT = FORCE_LOW;		// 9:8    Action on compare output pin 11 CMP6
	EvbRegs.ACTRB.bit.CMP12ACT = FORCE_LOW;		// 11:10  Action on compare output pin 12 CMP6	
	EvbRegs.ACTRB.bit.D = 0;					// 14:12  Basic state vector bits	
	EvbRegs.ACTRB.bit.SVRDIR = 0;				// 15     Space vecor PWM rotation dir
	// ----- Set up and load DBTCONx, if dead-band is to be used -----
	
	// ----- Initialize CMPRx -----
	EvaRegs.CMPR1 = TX_TPR/3;		// Ping the PWM1&2 output pins when T1CNT >= 1/3 Period
	EvaRegs.CMPR2 = TX_TPR*2/3;		// Ping the PWM3&4 output pins when T1CNT >= 2/3 Period
	
	
	// ----- Set up COMCONA -----
	#define LD_UNDERFLOW	0	// 00 When T3CNT = 0 (underflow)
	#define	LD_UF_PERIOD	1	// 01 When T3CNT = 0 (underflow) or T3CNT = T3PR (period match)
	#define	LD_IMMEDIATE	2	// 10 Immediately
	#define	LD_RSVD 		3	// 11 Reserved; result is unpredictable
	EvaRegs.COMCONA.bit.FCOMPOE = 1;      		// 9      	Full Compare output enable
    EvaRegs.COMCONA.bit.ACTRLD = LD_UNDERFLOW;	// 11:10  	Action control register reload
   	EvaRegs.COMCONA.bit.SVENABLE = 0;     		// 12 		Disable Space vector PWM Mode 
	EvaRegs.COMCONA.bit.CLD = LD_UNDERFLOW;		// 14:13  	Compare register reload condition
	EvaRegs.COMCONA.bit.CENABLE = 1;			// 15 		Compare Enable     1=Enable compare operation
	
	// ----- Set up COMCONB -----
	EvbRegs.COMCONB.bit.FCOMPOE = 1;      		// 9      	Full Compare output enable
    EvbRegs.COMCONB.bit.ACTRLD = LD_UNDERFLOW;	// 11:10  	Action control register reload
   	EvbRegs.COMCONB.bit.SVENABLE = 0;     		// 12 		Disable Space vector PWM Mode 
	EvbRegs.COMCONB.bit.CLD = LD_UNDERFLOW;		// 14:13  	Compare register reload condition
	EvbRegs.COMCONB.bit.CENABLE = 1;			// 15 		Compare Enable     1=Enable compare operation
			
	// ----- Set up T1CON -----  
/*
//	#define	T1CON_INIT		0x1042			// Continuous upcount, T1Clock=DSPclock/1,enable timer,internal clock,reload when counter=0,enable timer compare,
	#define	T1CON_INIT		0x0842			// Continuous up/down count, T1Clock=DSPclock/1,enable timer,internal clock,reload when counter=0,enable timer compare,
	#define	T1PR_INIT		(1<<DAC_SIZE)	//

//	#define	T2CON_INIT		0x1 342			//	Set T2 clock at DSP clock / 8
//	#define	T2PR_INIT		(DSP_FREQ/8/TINTS_PER_SEC)	// 1 kHz period clock

	#define	T2CON_INIT		0x1042			// Continuous up-count, HSPCLK/1, enable, source=HSPCLK, enable timer compare  

	EvaRegs.T1CON.all = T1CON_INIT;		// Configure timer 1 and start it running	
*/

// FREE,SOFT = 00 Stop immediately on emulation suspend
// FREE,SOFT = 01 Stop after current timer period is complete on emulation suspend
// FREE,SOFT = 10 Operation is not affected by emulation suspend
// FREE,SOFT = 11 Operation is not affected by emulation suspend
	
	EvaRegs.T1CON.all = 0;

	#define	TCLKS_INT	0	// 00 Internal (HSPCLK)
	#define TCLKS_EXT	1	// 01 External (TCLKINx)
	#define	TCLKS_RSVD	2	// 10 Reserved
	#define	TCLKS_QEP	3	// 11 QEP circuit
	
	#define	TPS_DIV1	0	// 000 x/1
	#define	TPS_DIV2	1	// 001 x/2
	#define	TPS_DIV4	2	// 010 x/4
	#define	TPS_DIV8	3	// 011 x/8
	#define	TPS_DIV16	4	// 100 x/16
	#define	TPS_DIV32	5	// 101 x/32
	#define	TPS_DIV64	6	// 110 x/64
	#define	TPS_DIV128	7	// 111 x/128 (x = HSPCLK)
	
	#define	TMODE_STOP			0	// 00 Stop/Hold
	#define	TMODE_CONT_UPDOWN	1	// 01 Continuous-Up/-Down Count Mode
	#define	TMODE_CONT_UP		2	// 10 Continuous-Up Count Mode
	#define	TMODE_DIR_UPDOWN	3	// 11 Directional-Up/-Down Count Mode
//   EvaRegs.T1CON.bit.TECMPR:1;		// 1     Timer compare enable
//   EvaRegs.T1CON.bit.TCLD10:2;        // 3:2   Timer compare register reload
	EvaRegs.T1CON.bit.TCLKS10 = TCLKS_INT;		// 5:4   Clock source select = Internal (HSPCLK)
	EvaRegs.T1CON.bit.TPS = TPS_DIV1;			// 10:8  Input clock prescaler = HSPCLK/1
	EvaRegs.T1CON.bit.TMODE = TMODE_CONT_UPDOWN;// 12:11 Count mode selection = continuous up/down

	// ----- Set up T2CON -----  
	EvaRegs.T2CON.all = 0;
	
	EvaRegs.T2CON.bit.SET1PR = 0;			// 0     Period register select (0= Use T2PR, 1 = Use T1PR)
//   EvaRegs.T2CON.bit.TECMPR:1;        // 1     Timer compare enable
//   EvaRegs.T2CON.bit.TCLD10:2;        // 3:2   Timer compare register reload
	EvaRegs.T2CON.bit.TCLKS10 = TCLKS_INT;	// 5:4   Clock source select = Internal (HSPCLK)
//?	EvaRegs.T2CON.bit.T2SWT1  = 1;        	// 7     1= Start Timer2 with T1CON TENABLE bit
	EvaRegs.T2CON.bit.TPS = TPS_DIV1;		// 10:8  Input clock prescaler = HSPCLK/1
	EvaRegs.T2CON.bit.TMODE = TMODE_CONT_UP;// 12:11 Count mode selection = continuous up

	// ----- Set up T3CON -----  
	EvbRegs.T3CON.all = 0;
	EvbRegs.T3CON.bit.TCLKS10 = TCLKS_INT;		// 5:4   Clock source select = Internal (HSPCLK)
	EvbRegs.T3CON.bit.TPS = TPS_DIV1;			// 10:8  Input clock prescaler = HSPCLK/1
	EvbRegs.T3CON.bit.TMODE = TMODE_CONT_UPDOWN;// 12:11 Count mode selection = continuous up/down


	// ----- Enable EV timers  -----
	EvaRegs.T1CON.bit.TENABLE = 1;	//Enable EV Timer1   
	EvaRegs.T2CON.bit.TENABLE = 1;	//Enable EV Timer2
	EvbRegs.T3CON.bit.TENABLE = 1;	//Enable EV Timer3
//	EvbRegs.T4CON.bit.TENABLE = 1;	//Enable EV Timer4


}

