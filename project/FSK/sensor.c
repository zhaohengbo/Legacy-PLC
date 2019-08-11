//==========================================================================================
// Filename:		sensor.c
//
// Description:		Functions related to the sensing capabilities on the board.
//
// Copyright (C) 2000 - 2002 Texas Instruments Incorporated
// Texas Instruments Proprietary Information
// Use subject to terms and conditions of TI Software License Agreement
//
// Revision History:
// 05/20/02	HEM		Started file for CAN project from Jervis project.
// 11/17/04	HEM		Removed unused function ReadSmoothedSensor().
// 					Removed unused variables left over from CAN project.
//==========================================================================================

#include "main.h"

// Functions that will be run from RAM need to be assigned to 
// a different section.  This section will then be mapped using
// the linker cmd file.
#ifdef __cplusplus			// "C++"
#pragma CODE_SECTION("ramfuncs"); 
#else						// "C"
#pragma CODE_SECTION(ArmAllSensors, "ramfuncs");
#pragma CODE_SECTION(SmoothSensor, "ramfuncs");
#endif

//==========================================================================================
// Local function prototypes
//==========================================================================================


//==========================================================================================
// External Variables
//==========================================================================================
extern u16	uADCTimeoutCnt;	// Count of timeouts while waiting for ADC sequencer to finish 


//==========================================================================================
// Local constants
//==========================================================================================
//	#define	OVERSAMPLE_RATE	16
//	#define	REMOVE_HILO		(~True)

//	#define	OVERSAMPLE_RATE	1
//	#define	REMOVE_HILO		(~True)
	
//	#define	OVERSAMPLE_RATE	2
//	#define	REMOVE_HILO		(~True)
 	
// IMPORTANT!  This value must match value assigned in subs.asm
//	#define	OVERSAMPLE_RATE	10
	#define	OVERSAMPLE_RATE			4
	    asm("OVERSAMPLE_RATE .set	4");
	#define	REMOVE_HILO		True
//	#define	REMOVE_HILO		(~True)

//	#define	OVERSAMPLE_RATE	3
//	#define	REMOVE_HILO		True


// These clock constant must also be defined in assembly because they are used in the RPTNOP() assembly macro.

#define	HSPCPS		 1	// HiSpeed Peripheral Clock PreScale.  Ratio of DSP clock / HiSpeed Periph Clock
   asm("HSPCPS	.set 1"); 
						//!!! Move this to main.h and use it in InitSysCtrl().

// ADC Clock PreScale.  Ratio of HiSpeed Peripheral Clock / ADC Core Clock
// Allowed values: Min = 1, Max = 30.  Odd values other than 1 will round down.
// Additional requirement: Do not exceed 25 MHz.
#define	ADCPS		 6	// ADC Core Clock = HiSpeed Periph Clock / 6  = 25 MHz for 150 MHz HSPC clock
   asm("ADCPS	.set 6"); 
//#define ADCPS		 30	// ADC Core Clock = HiSpeed Periph Clock / 30 =  5 MHz for 150 MHz HSPC clock


// After changing certain ADC control registers, must wait this many DSP clock ticks for them to take effect
#define	ADC_CTRL_DELAY		 (4*HSPCPS*ADCPS)	// = 4 ADC clock cycles
   asm("ADC_CTRL_DELAY	.set (4*HSPCPS*ADCPS)"); 

//==========================================================================================
// Function:		ConfigureADCs()
//
// Description: 	This function powers up the analog circuitry for the ADCs.
//					It should be called during board initialization.
//
//	Assumptions:	
//
// Revision History:
// 05/21/02	HEM		New function.
// 06/12/02 HEM		Added analog circuit power-ups.
// 06/20/02 HEM		Changed ADC control delays to match clock settings.
//					Configured each bit field in ADCTRL3 individually.
//==========================================================================================
void	ConfigureADCs(void)
{
//	#define	ACQ_PS			16		// Acquisition prescaler = 16x longer
	#define	ACQ_PS			2		// Acquisition prescaler = 2x longer
//	#define	ACQ_PS			1		// Acquisition prescaler = fastest possible

//	#define	ADCTRL1_INIT	(0x3010|((ACQ_PS-1)<<8))	// Soft&free=1, Cascaded sequencers
	#define	ADCTRL1_INIT	(0x3000|((ACQ_PS-1)<<8))	// Soft&free=1, Separate sequencers	
	#define	ADCTRL1_RESET	0x4000	// Reset sequencers
	



	AdcRegs.ADCTRL1.all = ADCTRL1_RESET;	// Reset the converters and sequencer
	RPTNOP(ADC_CTRL_DELAY);					// Wait for control change to take effect

	AdcRegs.ADCTRL1.all = ADCTRL1_INIT;		// Take converters out of reset and configure it
	RPTNOP(ADC_CTRL_DELAY);					// Wait for control change to take effect

	AdcRegs.ADCTRL3.bit.SMODE_SEL = 0;		// Set ADC Sampling Mode:  0=Sequential, 1=Simultaneous
//?	RPTNOP(ADC_CTRL_DELAY);					// Wait for control change to take effect (needed?)

	AdcRegs.ADCTRL3.bit.ADCCLKPS= (ADCPS/2);// Set ADC Clock Prescale
//?	RPTNOP(ADC_CTRL_DELAY);					// Wait for control change to take effect (needed?)

	AdcRegs.ADCTRL3.bit.ADCBGRFDN = 0x3;	// Power up Vref and Bandgap
	DelayNus(5000);							// Wait 5 ms 

	AdcRegs.ADCTRL3.bit.ADCPWDN = 0x1;		// Power up rest of ADC circuits
	DelayNus(20);							// Wait 20 us



	AdcRegs.ADCCHSELSEQ4.all = 0xFEDC;
 	AdcRegs.ADCCHSELSEQ3.all = 0xBA98;		  	
 	AdcRegs.ADCCHSELSEQ2.all = 0x7654;
	AdcRegs.ADCCHSELSEQ1.all = 0x3210;

	AdcRegs.ADCMAXCONV.all = 16-1;   		// Set number of samples to collect


	// Initializations from vardefs.h   **** MARK HEMINGER
	uADCTimeoutCnt = 0;
}




//==========================================================================================
// Function:		ReadAllSensors()
//
// Description: 	This function configures the A/D converters and sequencers to read all 
//					the sensors in a rapid burst.
//
//	Assumptions:	
//
// Revision History:
// 05/13/04	HEM		New function.

//==========================================================================================
void ReadAllSensors(u16 uWaitFlag)
{

//	u16* upADCResult;			// Pointer into ADC results array
	u16	uADCWaitCntr;			// Loop counter for ADC wait loop
//	u16	i;						// Generic loop counter  //!!!DEBUG
	
//	u16	uAvg;					// Averaged sensor reading

  // 	static u16	uAvgAll;		//!!!DEBUG Average of all values combined

	

	#define	ADCTRL2_RESET	0x4040	// Reset sequencers 1 & 2
	#define	ADCTRL2_INIT	0x2000	// Start conversion

	// Wait	this many microseconds for the ADC sequence to complete before reporting ADC Timeout.  
	// This formula is not very precise, but it does include the effect of the peripheral
	// clock and ADC clock dividers as well as the number of samples being collected.
	#define MAX_ADC_WAIT	(((OVERSAMPLE_RATE*HSPCPS*ADCPS)/5)+1)	


	//SetXF();					// Turn on XF flag for debug aid

/*    
 	AdcRegs.ADCCHSELSEQ4.all = 0xFEDC;
 	AdcRegs.ADCCHSELSEQ3.all = 0xBA98;		  	
 	AdcRegs.ADCCHSELSEQ2.all = 0x7654;
	AdcRegs.ADCCHSELSEQ1.all = 0x3210;

	AdcRegs.ADCMAXCONV.all = 16-1;   // Set number of samples to collect
*/

    if (uWaitFlag & 2)
	{
		// Poll until ADC sequence is done
		uADCWaitCntr = 0;
		while (AdcRegs.ADCST.bit.SEQ1_BSY == 1)	// wait for SEQ1_BSY bit to clear
		{
			if (uADCWaitCntr++ > MAX_ADC_WAIT)	// if ADC times out, increment counter and bail.
			{			
				uADCTimeoutCnt++;		
				break;
			}
	
			Delay1us();	// Wait 1 us for sequencer to complete
		} 
     	NOP; 		//!!!DEBUG
	}
 
    
//	AdcRegs.ADCMAXCONV.all = OVERSAMPLE_RATE-1;   // Set number of samples to collect

	AdcRegs.ADCTRL2.all |= ADCTRL2_RESET;		//aaa Reset the sequencers
	RPTNOP(ADC_CTRL_DELAY);						// Wait for control change to take effect

	AdcRegs.ADCTRL2.all &= !(ADCTRL2_RESET);	//aaa Turn off the reset
	RPTNOP(ADC_CTRL_DELAY);						// Wait for control change to take effect

	AdcRegs.ADCST.all = 0x0030;					// Clear the INT_SEQ1 & INT_SEQ2 interrupts
	AdcRegs.ADCTRL2.all = ADCTRL2_INIT;			// Start the conversion
    RPTNOP(ADC_CTRL_DELAY);						// DO NOT REMOVE THIS WAIT!!!  Must wait before checking status of SEQ1_BSY


	if (uWaitFlag  & 1)
	{
		// Poll until ADC sequence is done
		uADCWaitCntr = 0;
		while (AdcRegs.ADCST.bit.SEQ1_BSY == 1)	// wait for SEQ1_BSY bit to clear
		{
			if (uADCWaitCntr++ > MAX_ADC_WAIT)	// if ADC times out, increment counter and bail.
			{			
				uADCTimeoutCnt++;		
				break;
			}
	
			Delay1us();	// Wait 1 us for sequencer to complete
		} 
     	NOP; 		//!!!DEBUG
	}
    
 

	//ClearXF();								// Turn off XF flag for debug aid
	return;
}
  

//==========================================================================================
// Function:		ArmAllSensors()
//
// Description: 	This function configures the A/D converters and sequencers to read all 
//					the sensors in a rapid burst, triggered by the Event Manager
//
//	Assumptions:	
//
// Revision History:
// 05/18/04	HEM		New function.
// 08/11/04	HEM		Remove unnecessary delays
//==========================================================================================
void ArmAllSensors(void)
{

	#define	ADCTRL2_RESET_SEQ	0x4040	// Reset sequencers 1 & 2
//	#define	ADCTRL2_ARM_SEQ		0x0100	// Arm to start conversion when Event Manager triggers it	

	#define	ADCTRL2_ARM_SEQ		0x0900	// Arm to start conversion when Event Manager triggers it
										// Enable SEQ1 Interrupt to CPU

// 	AdcRegs.ADCCHSELSEQ4.all = 0xFEDC;
// 	AdcRegs.ADCCHSELSEQ3.all = 0xBA98;		  	
// 	AdcRegs.ADCCHSELSEQ2.all = 0x7654;
//	AdcRegs.ADCCHSELSEQ1.all = 0x3210;

	AdcRegs.ADCCHSELSEQ1.all = 0x6666;
#if	OVERSAMPLE_RATE > 4	
	AdcRegs.ADCCHSELSEQ2.all = 0x6666;
	#if	OVERSAMPLE_RATE > 8	
		AdcRegs.ADCCHSELSEQ3.all = 0x6666;
		#if OVERSAMPLE_RATE >12
			AdcRegs.ADCCHSELSEQ4.all = 0x6666;
		#endif
	#endif
#endif
			
	AdcRegs.ADCMAXCONV.all = OVERSAMPLE_RATE-1;   //!!!DEBUG!!! Set number of samples to collect

	AdcRegs.ADCTRL2.all |= ADCTRL2_RESET_SEQ;		//aaa Reset the sequencers
//??? NEEDED?	RPTNOP(ADC_CTRL_DELAY);						// Wait for control change to take effect

//	SetLED(CANRX_LED,  1);		//!!!DEBUG!!! Turn on RX LED
	
	AdcRegs.ADCST.all = 0x0030;					// Clear the INT_SEQ1 & INT_SEQ2 interrupts
	AdcRegs.ADCTRL2.all = ADCTRL2_ARM_SEQ;		// Arm the conversion.  Will start when EVT triggers it.

	return;
}


//==========================================================================================
// Function:		SmoothSensor()
//
// Description: 	This function averages multiple ADC readings into a single value.
//					Optionally, the highest and lowest values may be removed before averaging
//					to eliminate noise spikes.
//
//	Assumptions:	ADC conversion was started by some other function
//					ADC conversion is complete
//					Results are stored in multiple locations starting at ADCRESULT0.
//
// Revision History:
// 08/04/04	HEM		New function, extracted from ReadSmoothedSensor().
// 10/19/04	HEM		Moved into standalone functions.
//==========================================================================================
u16 SmoothSensor(void)
{
	u16* upADCResult;			// Pointer into ADC results array
	u16	i;						// Generic loop counter  
	u32	ulSum;					// Sum of all results
	u16	uAvg;					// Averaged sensor reading

#if	(REMOVE_HILO == True)	// ==== Remove highest and lowest readings and average what's left ====
//  static u16	uAvgAll;		//!!!DEBUG Average of all values combined
//	static u16	uHigh;			// Highest value in ADC results set
//	static u16  uLow;			// Lowest value in ADC results set   
	u16	uHigh;					// Highest value in ADC results set
	u16 uLow;					// Lowest value in ADC results set  
	 
   	uHigh = 0;					// Reset highest value in ADC results set
	uLow = 65535;				// Reset lowest value in ADC results set   
 	
 	ulSum = 0;					// Reset running total     	

	upADCResult = (u16*)&(AdcRegs.ADCRESULT0);	// Pointer into ADC results array
	for (i=0; i<OVERSAMPLE_RATE; i++)
	{
		uHigh = Max(uHigh, *upADCResult);		// Find highest value in list
		uLow = Min(uLow, *upADCResult);			// Find lowest value in list
//		uHigh = Max16(uHigh, *upADCResult);		// Find highest value in list
//		uLow = Min16(uLow, *upADCResult);		// Find lowest value in list
		ulSum += *upADCResult++;				// Add value to running sum
	}
//	uAvgAll = ulSum / OVERSAMPLE_RATE;			// DEBUG: Calc avg without high/low stripped out for comparison
	ulSum = ulSum - uHigh - uLow;				// Strip out the highest and lowest samples
	uAvg =  ulSum / (OVERSAMPLE_RATE-2); 

#else						// ==== Average all the measurements together into one result ====
 	ulSum = 0;									// Reset running total    	

	upADCResult = (u16*)&(AdcRegs.ADCRESULT0);	// Pointer into ADC results array
	for (i=0; i<OVERSAMPLE_RATE; i++)
	{
		ulSum += *upADCResult++;				// Add value to running sum
	}	   

	uAvg = ulSum / OVERSAMPLE_RATE;				// Calculate average

#endif

	return(uAvg);
}








