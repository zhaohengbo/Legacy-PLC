//==========================================================================================
// Filename:		dacout.c
//
// Description:		Functions related to the mirror current control DACs on the board.
//
// Copyright (C) 2000 - 2001 Texas Instruments Incorporated
// Texas Instruments Proprietary Information
// Use subject to terms and conditions of TI Software License Agreement
//
// Revision History:
// 08/20/01	EGO		Started file.  From owlink1 project.
// 11/xx/04	HEM		Comment out unused functions WriteAuxDAC() and RampPWMDACs().
//					Eliminated ConfigurePWMDAC().
//==========================================================================================

#include "main.h"


//==========================================================================================
// Local function prototypes
//==========================================================================================
void RampPWMDACs(q16 qOutRange, q16 qOutStepSize);


//==========================================================================================
// External Variables
//==========================================================================================


//==========================================================================================
// Local constants
//==========================================================================================


//==========================================================================================
// Function:		WritePWMDAC()
//
// Description: 	This function sends a requested value to the PWM DACs.
// 					Outputs are differential, treated like sign+magnitude.
//					Odd-numbered axes are driven with opposite sign to reduce power-supply 
//					surges.
//
// Revision History:
// 05/13/02 HEM		New function.
// 05/21/02 HEM		Clean up dead code.
// 06/28/02 HEM		Changed from individual bits back to entire words to fix erratic PWM
//					response.  Have since found that changing COMCON to immediate output may
//					be enough to fix problem, so we might be able to undo this change later.
// 07/31/02 HEM		Swapped TEC0 & 1 with TEC2 & 3 to match new board wiring.
// 08/08/02 HEM		Added a small ramp dither that will affect the LSB of the DAC output.
// 08/23/02 HEM		Instead of ramp dither, keep track of the truncated fractional part.
//==========================================================================================
void WritePWMDAC(u16 uChan, q16 qOutVal)
{
 	static q16	qFrac[NUM_CHANNELS] = {0, 0, 0, 0};	// Fractional part not sent
 	q16			qDACVal;	// Integer value to be sent to DAC.  Less resolution than OutVal
 	
 	NOP;	// Don't remove these two NOPs!!!  Don't know why, but without them, every 
  	NOP;	// 8th and 9th servo periods are bad.

/*
	// Add last three bits of sample number just below the LSB. This will modulate the 
	// LSB of the output to give the same average value as if we had three more bits
	// of range in the DAC.
	qOutVal = qOutVal + ((uSampleNumber & 0x07)<<(Q_CURR-DAC_SIZE-3)) -3;	// Extend resolution by 3 bits
//	qOutVal = qOutVal + ((uSampleNumber & 0x03)<<(Q_CURR-DAC_SIZE-2)) -1;	// Extend resolution by 2 bits
//	qOutVal = qOutVal + ((uSampleNumber & 0x01)<<(Q_CURR-DAC_SIZE-1)) -0;	// Extend resolution by 1 bit
*/
	// 
#if DAC_SIZE <= 14
	qDACVal = (qOutVal+qFrac[uChan]) >>(14-DAC_SIZE);		// Add fractional part, then truncate
	qFrac[uChan] = (qOutVal+qFrac[uChan]) - (qDACVal<<(14-DAC_SIZE));		// Determine fractional part
#else
	qDACVal = (qOutVal+qFrac[uChan]) <<(DAC_SIZE-14);		// Add fractional part, then truncate
	qFrac[uChan] = (qOutVal+qFrac[uChan]) - (qDACVal>>(DAC_SIZE-14));		// Determine fractional part
#endif
	
	switch	(uChan)
	{	
		case 0:	// Active high output.  Send + values on PWM7 and - values on PWM8
		{
			if (qOutVal >= 0)
			{
				// ACTRB = (ACTRB & ~0x000F) | 0x0001 ;	// PWM7= Active Lo, PWM8 = 0
				EvbRegs.ACTRB.all = (EvbRegs.ACTRB.all & ~0x000F) | 0x0001 ;	// PWM7= Active Lo, PWM8 = 0
				//???EvbRegs.ACTRB.bit.CMP7ACT = 0x1;		// PWM7 = Active Lo
				//???EvbRegs.ACTRB.bit.CMP8ACT = 0x0;		// PWM8 = 0
//				EvbRegs.CMPR4 = qOutVal>>(14-DAC_SIZE);
				EvbRegs.CMPR4 = qDACVal;
				
			}
			else
			{
				// ACTRB = (ACTRB & ~0x000F) | 0x0004 ;	// PWM7= 0 , PWM8 = Active Lo
				EvbRegs.ACTRB.all = (EvbRegs.ACTRB.all & ~0x000F) | 0x0004 ;	// PWM7= 0 , PWM8 = Active Lo
				//???EvbRegs.ACTRB.bit.CMP7ACT = 0x0;		// PWM7 = 0
				//???EvbRegs.ACTRB.bit.CMP8ACT = 0x1;		// PWM8 = Active Lo
//				EvbRegs.CMPR4 = (-qOutVal)>>(14-DAC_SIZE);
				EvbRegs.CMPR4 = -qDACVal;
			}
			break;
		}

		case 1:	// Active low output, so send 1.00-value.  Send + values on PWM9 and - values on PWM10
		{
			if (qOutVal >= 0)
			{
				// ACTRB = (ACTRB & ~0x00F0) | 0x0020 ;		// PWM9= Active Hi, PWM10 = 0
				EvbRegs.ACTRB.all = (EvbRegs.ACTRB.all & ~0x00F0) | 0x0020 ;	// PWM9= Active Hi, PWM10 = 0
				//???EvbRegs.ACTRB.bit.CMP9ACT  = 0x2;		// PWM9 = Active Hi
				//???EvbRegs.ACTRB.bit.CMP10ACT = 0x0;		// PWM10 = 0
//				EvbRegs.CMPR5 = ((1<<14)-qOutVal)>>(14-DAC_SIZE);
				EvbRegs.CMPR5 = (1<<DAC_SIZE)-qDACVal;		// Send (1.0-val)
				
			}
			else
			{
				// ACTRB = (ACTRB & ~0x00F0) | 0x0080 ;		// PWM9= 0, PWM10 = Active Hi
				EvbRegs.ACTRB.all = (EvbRegs.ACTRB.all & ~0x00F0) | 0x0080 ;	// PWM9= 0, PWM10 = Active Hi
				//???EvbRegs.ACTRB.bit.CMP9ACT  = 0x0;		// PWM9 = 0
				//???EvbRegs.ACTRB.bit.CMP10ACT = 0x2;		// PWM10 = Active Hi
//				EvbRegs.CMPR5 = ((1<<14)+qOutVal)>>(14-DAC_SIZE);	// Send (1.0-(-val))
				EvbRegs.CMPR5 = (1<<DAC_SIZE)+qDACVal;		// Send (1.0-(-val))
				
			}
			break;
		}

		case 2:	// Active high output.  Send + values on PWM1 and - values on PWM2
		{
			if (qOutVal >= 0)
			{
				// ACTRA = (ACTRA & ~0x000F) | 0x0001 ;	// PWM1= Active Lo, PWM2 = 0
				EvaRegs.ACTRA.all = (EvaRegs.ACTRA.all & ~0x000F) | 0x0001; 	// PWM1= Active Lo, PWM2 = 0
				//???EvaRegs.ACTRA.bit.CMP1ACT = 0x1;			// PWM1 = Active Lo
				//???EvaRegs.ACTRA.bit.CMP2ACT = 0x0;			// PWM2 = 0
//				EvaRegs.CMPR1 = qOutVal>>(14-DAC_SIZE);		// Send + value
				EvaRegs.CMPR1 = qDACVal;						// Send + value
			}
			else
			{
				// ACTRA = (ACTRA & ~0x000F) | 0x0004 ;	// PWM1= 0 , PWM2 = Active Lo
				EvaRegs.ACTRA.all = (EvaRegs.ACTRA.all & ~0x000F) | 0x0004 ;	// PWM1= 0 , PWM2 = Active Lo
				//???EvaRegs.ACTRA.bit.CMP1ACT = 0x0;			// PWM1 = 0
				//???EvaRegs.ACTRA.bit.CMP2ACT = 0x1;			// PWM2 = Active Lo
//				EvaRegs.CMPR1 = (-qOutVal)>>(14-DAC_SIZE);	// Send - value
				EvaRegs.CMPR1 = -qDACVal;						// Send - value
			}
			break;
		}

		case 3:	// Active low output, so send 1.00-value.  Send + values on PWM3 and - values on PWM4
		{	

			if (qOutVal >= 0)
			{
				// ACTRA = (ACTRA & ~0x00F0) | 0x0020 ;		// PWM3= Active Hi, PWM4 = 0
				EvaRegs.ACTRA.all = (EvaRegs.ACTRA.all & ~0x00F0) | 0x0020 ;	// PWM3= Active Hi, PWM4 = 0
				//??EvaRegs.ACTRA.bit.CMP3ACT = 0x2;			// PWM3 = Active Hi
				//??EvaRegs.ACTRA.bit.CMP4ACT = 0x0;			// PWM4 = 0
//				EvaRegs.CMPR2 = ((1<<14)-qOutVal)>>(14-DAC_SIZE);	// Send (1.0-val)
				EvaRegs.CMPR2 = (1<<DAC_SIZE)-qDACVal;			// Send (1.0-val)
			}	
			else
			{
				// ACTRA = (ACTRA & ~0x00F0) | 0x0080 ;		// PWM3= 0, PWM4 = Active Hi
				EvaRegs.ACTRA.all = (EvaRegs.ACTRA.all & ~0x00F0) | 0x0080 ;	// PWM3= 0, PWM4 = Active Hi
				//???EvaRegs.ACTRA.bit.CMP3ACT = 0x0;			// PWM3 = 0
				//???EvaRegs.ACTRA.bit.CMP4ACT = 0x2;			// PWM4 = Active Hi
//				EvaRegs.CMPR2 = ((1<<14)+qOutVal)>>(14-DAC_SIZE);	// Send (1.0-(-val))
				EvaRegs.CMPR2 = (1<<DAC_SIZE)+qDACVal;			// Send (1.0-(-val))
			}
			break;
		}

	}	// end of switch statement

 	NOP;	// Don't remove this NOP!!!  Don't know why, but without it, every 8th servo sample is bad
	return;
}

/*
//==========================================================================================
// Function:		WriteAuxDAC()
//
// Description: 	This function sends a requested value to the setpoint DACs.
// 					Outputs are single-ended, from 0 to Vdacref.
//					There are not enough free DAC channels of the same type to run them all
//					 the same way, so we use different types for the odd and even channels.
//					Even-numbered channels use the PWM output directly from the timers.
//					Odd-numbered channels use the PWM compare units.
//
// Revision History:
// 05/20/02 HEM		New function.
//==========================================================================================
void WriteAuxDAC(u16 uChan, q16 qOutVal)
{
 	NOP;	// Don't remove this NOP!!!  Don't know why, but without it, every 8th servo sample is bad

	switch	(uChan)
	{	
		case 0:	// Send output from Timer 1 compare output
		{
			EvaRegs.T1CMPR = qOutVal + (1<<(DAC_SIZE-1));	
			break;
		}

		case 1:	// Send output from CMP5 pin.
		{
			// ACTRA = (ACTRA & ~0x0F00) | 0x0100 ;		// CMP5= Active Lo, CMP6 = 0
			EvaRegs.ACTRA.bit.CMP5ACT = 0x1;			// CMP5 = Active Lo
			EvaRegs.ACTRA.bit.CMP6ACT = 0x0;			// CMP6 = 0
			EvaRegs.CMPR3 = qOutVal + (1<<(DAC_SIZE-1));
			break;
		}

		case 2:	// Send output from Timer 3 compare output
		{
			EvbRegs.T3CMPR = qOutVal + (1<<(DAC_SIZE-1));	
			break;
		}

		case 3:	// Send output from CMP11 pin.
		{
			// ACTRB = (ACTRB & ~0x0F00) | 0x0100 ;		// CMP11= Active Lo, CMP12 = 0
			EvbRegs.ACTRB.bit.CMP11ACT = 0x1;			// CMP11 = Active Lo
			EvbRegs.ACTRB.bit.CMP12ACT = 0x0;			// CMP12 = 0
			EvbRegs.CMPR6 = qOutVal + (1<<(DAC_SIZE-1));
			break;
		}

	}	// end of switch statement

 	NOP;	// Don't remove this NOP!!!  Don't know why, but without it, every 8th servo sample is bad
	return;
}
*/

/*
//==========================================================================================
// Function:		RampPWMDACs()
//
// Description: 	Ramps all 4 TEC PWM DACs through their range of voltage.
//					Leaves them set at zero.
//					Waveform looks like this:			
// 												  /\	
// 												 /  \
// 												/    \    __
// 												      \  /
// 												       \/
//
// Revision History:
// 05/14/01 HEM		New function.
// 06/28/02 HEM		Added more segments to eliminate discontinuities.
//==========================================================================================
void	RampPWMDACs(q16 qOutRange, q16 qOutStepSize)
{
	u16	uChan;
	q16 qOutVal;
	
	#define RAMP_DELAY	5	// 5 us per step while ramping
	
	// Ramp DAC value through requested range, from zero to positive	
	for (qOutVal= 0; qOutVal<qOutRange; qOutVal += qOutStepSize)
	{
		for (uChan=0; uChan<NUM_CHANNELS; uChan++)
		{
			WritePWMDAC(uChan, qOutVal);
		}
		DelayNus(RAMP_DELAY);	// Wait
		uSampleNumber++; 
	}

	// Ramp DAC value through requested range, from positive to negative 
	for (qOutVal= -qOutRange; qOutVal<qOutRange; qOutVal += qOutStepSize)
	{
		for (uChan=0; uChan<NUM_CHANNELS; uChan++)
		{
			WritePWMDAC(uChan, -qOutVal);
		}
		DelayNus(RAMP_DELAY);	// Wait
		uSampleNumber++;
	}

	// Ramp DAC value through requested range, from negative to zero	
	for (qOutVal= -qOutRange; qOutVal<0; qOutVal += qOutStepSize)
	{
		for (uChan=0; uChan<NUM_CHANNELS; uChan++)
		{
			WritePWMDAC(uChan, qOutVal);
		}
		DelayNus(RAMP_DELAY);	// Wait 
		uSampleNumber++;
	}
	
	// Set coil voltage back to zero
	qOutVal = 0;
	for (uChan=0; uChan<NUM_CHANNELS; uChan++)
	{
		WritePWMDAC(uChan, qOutVal);
	}
	DelayNus(RAMP_DELAY*500);	// 


	return;

}
*/

