//==========================================================================================
// Filename:		dataDet.c
//
// Description:		Functions for data detection in an OFDM power line modem.
//
// Copyright (C) 2000 - 2003 Texas Instruments Incorporated
// Texas Instruments Proprietary Information
// Use subject to terms and conditions of TI Software License Agreement
//
// Revision History:
// 01Feb05	Hagen	new file
// 22Feb05	Hagen	archived in Visual Source Save
// 24Feb05	Hagen	added comments in runpll()
//========================================================================
#if 1==0
	#include "psk_modem.h" 
#else
	#include "datadet.h"         
#endif
#include <string.h>						// contains memset()

//#define	TX_BIT_COUNT		42		// Arefeen added on 062505
#define SAM_PER_BIT			TX_BIT_COUNT // number of samples of downsampled carrier per bit

#define FIND_WORDSYNC_TO	24			// timeout after N bit times
#define FIND_EOP_TO			(8*128)		// timeout at 128 bytes

#define VHYST_THRS			100   		// vertical bits
#define THYST_THRS			4      		// time samples
#define BIT_DET_THRS		(SAM_PER_BIT/2)		// sample detBit at this point in bit window
#define BIT_WIN_TOL			3			// amount to wait to see if a bit transition arrives
#ifdef DSP_COMPILE
	// Functions that will be run from RAM need to be assigned to 
	// a different section.  This section will then be mapped using
	// the linker cmd file.
	#ifdef __cplusplus			// "C++"
		#pragma CODE_SECTION("ramfuncs"); 
	#else						// "C"
//		#pragma CODE_SECTION(runPLL, "ramfuncs"); // Arefeen commented on 062705
		#pragma CODE_SECTION(receive, "ramfuncs");
	#endif
#endif


#ifdef MEX_COMPILE
#define BITSYNC_PATTERN		0xAAAA
#define WORDSYNC_PATTERN	0x567B
#define WORDSYNC_PAT_NEG	(0xFFFF ^ WORDSYNC_PATTERN)
#define EOP_PATTERN			0x0733

//#define AGC_TABLE_LEN		32
//#define AGC_SCALE			5
//#define GAIN_INDEX_THRS		200
//#define GAIN_TABLE_SCALE	4

u32		sampleCount = 0;
u16		uRxMode = FIND_BITSYNC;
u16		uRxMsgPending = 0;

/*==========================================================================================
	Function:		processReadSamples()

	Description:	simulate ADC interupt 	

	Revision History:
==========================================================================================*/
u16 processReadSamples( dCplxPtr rec)
{
	u16			n;
	s16			ADCsample;

	//---- init diag vector ------------------
	SaveTraceInit((u16)(10*rec.len));

	//---- set up for preamble detect -------------
	reset_to_BitSync();
	sampleCount = 0;
	uRxByteCount = 0;

	//---- loop through read data ------------
	for( n = 0; n < rec.len; n++ )
	{
		#ifdef MEX_COMPILE
			ADCsample = (s16)(*(rec.r++));
			sampleCount++;
		#else
		//---- get the ADC sample ------------------------------ 
			ADCsample = SmoothADCResults();	// Combine the results from A/D Converter, 
											// scale them, and convert them to signed values	
			ArmAllSensors();				// Re-arm the sensors for the next reading

			if (++ADCIntCount >= ADCINT_COUNT_MAX)
				ADCIntCount = 0;
		#endif

		receive(ADCsample);

	}

	SaveTraceWorkspace("pskDiag");

	return uRxByteCount;
}
#endif


/*==========================================================================================
Function:		receive()

Description: 	This function is called by the ADC INT routine.  It is passed the latest 
				receive signal ADC sample which it passes to runpll().  runpll() returns a 
				value representing the phase between the receive signal and the PLL generated 
				sine wave.  This phase value is called phcos (cosine derived phase).
				
				The phcos sequence is processed with voltage and time hysteresis "square up"
				the phase.  This squared-up phase is called detBit.  Varialbe detBit is used to
				detect the various data patterns depending on which mode the receive function is in.
				
Globals:		
	pllControl	pll; 							// PLL properties 
	u16			rxUserDataArray[MAX_RX_MSG_LEN];// byte-wide buffer for user data	

Revision History:
01Feb05	Hagen	Started wih Hemingers function FilterCorrRx().
15Feb05 Hagen	Added hysteresis
22Feb05	Hagen	Added EOP_HOLD_OFF state to receive function
28Feb05	Hagen	Put EOP timeout at beginning of case statements so that the modem
				can start to look for a packet right after receiving one, but wait to TX.
				Also changed FIND_BITSYNC bit window timeout to reset on too big and too small bit times
07Mar05	Hagen	added Parity error check
==========================================================================================*/
void receive(s16 demodSample)
{
	static u16	bitNum;					// count the bits in a byte
	static u16	detData  = 0;			// receive data word
	static u16  detBit = 0;				// squared-up version of phase data
	static s16  bitSample = False;		// flag used to sample detBit in FIND_DATA
	static u16 	uEOP_holdOffCnt = 0;	// time to wait before transmitting
//	static u16 	histCnt = 0;			// hysteresis counter
	static u16  hystCnt = 0;
	static s16	bitPhase = 0;			// counter representing the phase within a bit window  
	static s16	polarity;				// polarity of the data, based on detection of WORDSYNC

	u16			bitTransition = False;	// flag used to sample detBit in FIND_BITSYNC
	s16			diagSample = 0;			// flag used to generate trace data 


	//---- apply time and voltage hysteresis to the demod data to squar it up -------
	if( detBit == 1 )							// detBit is pos
	{
		if( demodSample < (-VHYST_THRS) )	// demodSample is neg
		{
			hystCnt++;
			if( hystCnt > THYST_THRS )
			{
				hystCnt = 0;
				detBit  = 0;					// transistion pos-neg
				bitTransition = True;
			}
		}
	}
	else										// detBit is neg
	{
		if( demodSample > VHYST_THRS )		// demodSample is pos
		{
			hystCnt++;
			if( hystCnt > THYST_THRS )
			{
				hystCnt = 0;
				detBit  = 1;					// transistion neg-pos
				bitTransition = True;
			}
		}
	}

	
	//---- look for information depending on state of modem -------------	
	switch( uRxMode	)
	{
	//---- look for bitSync, but don't transmit just yet ----------------
	case EOP_HOLD_OFF:
		uEOP_holdOffCnt--;					// countdown random TX holdoff time
		if( uEOP_holdOffCnt <= 0 )	
		{
			uRxMode = FIND_BITSYNC;			// OK to transmit now
		}		  
		// fall through to Find_BITSYNC

	//---- look for bitSync -------------------------------
	case FIND_BITSYNC:
		//uRxModeCount++;
		bitPhase++;
/*		if( bitTransition )
		{
			//---- reset if too long or too short between bit transitions -----------
			if( (bitPhase > (SAM_PER_BIT+11)) || (bitPhase < (SAM_PER_BIT-11)) )
				detData = 0;
			else
				detData = (detData << 1) | detBit;
						
			//uRxModeCount = 0;
			bitPhase = 0;
			diagSample = 1;
*/


		if( bitTransition )
		{
			bitPhase = 0;			// reset counter for phase inside bit window
		}
		if( bitPhase >= SAM_PER_BIT+11 )
		{
			bitPhase -= SAM_PER_BIT;  // reset counter for phase inside bit window
		}

		if( bitPhase == BIT_DET_THRS )
		{
			detData = (detData << 1) | detBit; // detect the data!
			diagSample = 1;


			//---- compare to preamble pattern -------------
   			if( detData == BITSYNC_PATTERN )
			{
				uRxMode = FIND_WORDSYNC;
				uRxModeCount = 0;
				//bitPhase = 0; // Arefeen uncommented on 062705
				bitSample = True;
				detData = 0;
				plcModeSnap = plcMode;
				//if( plcModeSnap == RX_MODE )	// just a place to put a breakpoint
				//{
				//	NOP;
				//	NOP;
				//}

	   			ulPlcStats[RX_PREDET_COUNT][plcModeSnap]++;		// count Preamble detections
				//SetLED(PLC_RX_BUSY_LED,  1);// Turn RX BUSY LED ON
				SetLED(PLC_RX_GOOD_LED,  0);// Turn RX GOOD LED OFF	

				#ifdef MEX_COMPILE
					#if MEX_VERBOSE
					mexPrintf("preamble found at sample %d\n", sampleCount);
					#endif
		   		#endif
			}
		}

		break;

	//---- look for WordSync -------------------------------
	case FIND_WORDSYNC:
		bitPhase++;

		if( bitTransition )
		{
			bitPhase = 0;			// reset counter for phase inside bit window
			//bitSample = True;		// enable detecting bit value
		}
		if( bitPhase >= SAM_PER_BIT+BIT_WIN_TOL )
		{
			bitPhase -= SAM_PER_BIT;  // reset counter for phase inside bit window
			//bitSample = True;		  // enable detecting bit value
		}

		//if( bitPhase >= BIT_DET_THRS )
		if( bitPhase == BIT_DET_THRS )
		{
			//if( bitSample )
			{
				//bitSample = False;		// disable detecting the bit after this
				detData = (detData << 1) | detBit; // detect the data!
				diagSample = 1;

				//---- look for WordSync ------------
				if( detData == WORDSYNC_PATTERN )
				{
					uRxMode = FIND_DATA;
					polarity = 0;
				}
				if( detData == WORDSYNC_PAT_NEG )
				{
					uRxMode = FIND_DATA;
					polarity = 1;
				}
				if( uRxMode == FIND_DATA )
				{
					uRxModeCount = 0;
					bitNum = CODEWORD_LEN;
					detData = 0;
					uRxByteCount = 0;

					SetLED(PLC_RX_BUSY_LED,  1);// Turn RX BUSY LED ON
					
		   			ulPlcStats[RX_SYNCDET_COUNT][plcModeSnap]++;	// Count WordSync detections
					#ifdef MEX_COMPILE
					#if MEX_VERBOSE
						mexPrintf("WordSync found at sample %d\n", sampleCount);
					#endif
					#endif
				}

				//---- timeout if too long in this state
				uRxModeCount++;
				if( uRxModeCount > FIND_WORDSYNC_TO )
				{
					reset_to_BitSync();
					detData = 0;
					ulPlcStats[RX_ERR_WORDSYNC_TO][plcModeSnap]++;	// Count WordSync timeouts
					#ifdef MEX_COMPILE
						#if MEX_VERBOSE
						mexPrintf("WordSync not found\n");
						#endif
					#endif
					break;
				}

			}		// if( bitSample )
		}			// if( bitTransition )

		break;

	//---- look for data --------------------------------------
	case FIND_DATA:
	case FIND_EOP:
		bitPhase++;

		if( bitTransition )
		{
			bitPhase = 0;			// reset counter for phase inside bit window
			//bitSample = True;		// enable detecting bit value
		}
		if( bitPhase >= SAM_PER_BIT+BIT_WIN_TOL )
		{
			bitPhase -= SAM_PER_BIT;  // reset counter for phase inside bit window
			//bitSample = True;		  // enable detecting bit value
		}

		//if( bitPhase >= BIT_DET_THRS )
		if( bitPhase == BIT_DET_THRS )
		{
			//if( bitSample )
			{
				bitSample = False;
				detData = (detData << 1) | (detBit^polarity);
				diagSample = 1;

				//---- process a byte of data ------------
				bitNum--;
				if( bitNum == 0 )
				{
					uRxModeCount++;
					if( uRxModeCount > FIND_EOP_TO )
					{
						reset_to_BitSync();
						detData = 0;
			   			ulPlcStats[RX_EOP_TIMEOUT][plcModeSnap]++;		// 
						#ifdef MEX_COMPILE
							#if MEX_VERBOSE
		 					mexPrintf("EOP not found.  state counter = %d\n", uRxModeCount );
							#endif
						#endif
						break;
					}

					if( uRxByteCount >= MAX_RX_MSG_LEN )
					{
						reset_to_BitSync();
						detData = 0;
			   			ulPlcStats[RX_MSGLEN_ERROR][plcModeSnap]++;		// 
						#ifdef MEX_COMPILE
							#if MEX_VERBOSE
		 					mexPrintf("RX message too long.\n");
							#endif
						#endif
						break;
					}

					//---- at the end? ----------------------------
					if( detData == EOP_PATTERN )
					{
						uRxMsgPending = True;	// Indicate received message pending.  
			   			ulPlcStats[RX_EOP_COUNT][plcModeSnap]++; 	// Count EOP patterns found 
						
						reset_to_BitSync(); 	// well, almost bitSync
						uRxMode = EOP_HOLD_OFF;	// initialize everything but wait to TX
						// calculate a ramdom hold off from ADC int counter 
						uEOP_holdOffCnt = 11*21*2 + ( (u16)(ulTimerIntCounter) & 0x7FF );
						
						#ifdef MEX_COMPILE
						#if MEX_VERBOSE
							mexPrintf("EOP found at sample %d\n", sampleCount);
						#endif
						#endif
					}

					//---- put each received byte into the receive buffer --------
					rxUserDataArray[uRxByteCount] = detData << 5;  // store data in upper byte					
					
					if( (uRxMode == FIND_DATA) && ( rxUserDataArray[uRxByteCount] != uTxPrecodeTable[detData>>3] ) )
					{
						ulPlcStats[RX_ERR_PARITY][plcModeSnap]++; 	// Count EOP patterns found 
					}
					
					uRxByteCount++;
					detData = 0;					// clear out detData for next byte
					bitNum = CODEWORD_LEN;			// this leaves parity in low byte

				}	// if( bitnum )
			}		// if( bitSample )
		}			// if( bitTransition )

		break;

	}	// switch block

	//---- diagnostics ---------------------
	#if TRACE_BUF_LEN > 0
		#ifdef DSP_COMPILE
			//SaveTrace( detBit );	 
			//SaveTrace( detData );	 
			SaveTrace( bitPhase );	   // 3rd trace var
			SaveTrace( (uRxMode<<2) + (bitTransition<<1) + detBit );  // 4th trace var
			//SaveTrace( (uRxMode<<2) + (diagSample<<1) + detBit );  // 4th trace var
		#endif	
		#ifdef MEX_COMPILE
			SaveTrace( pll.phaseHold );
			SaveTrace( pll.bitPhase );
			SaveTrace( detBit );
			SaveTrace( (uRxMode<<2) + diagSample );
		#endif
	#else
		diagSample = diagSample;
	#endif

	return;
}




/*==========================================================================================
Function:		reset_to_BitSync()

Description: 	Go back to uRxMode = FIND_BITSYNC
  				reset counters and such to start looking for packet all over again.
Globals:		
		pllControl	pll;    	// PLL properties
		u16			uRxMode; 	// receive state

 Revision History:
 01/28/05	Hagen	New Function
==========================================================================================*/
void reset_to_BitSync(void)
{
	uRxMode = FIND_BITSYNC;
	uRxModeCount = 0;
	bitPhase = 0;
	polarity = 0;
	hystCnt = 0;

	demod = 0;
	memset(demodBuf, 0, ADCINT_COUNT_MAX*sizeof(s16));
	
	
	
//for( n = 0; n < ADCINT_COUNT_MAX; n++ )
//{
//	demodBuf[n] = 0;
//}	

	//---- set up for preamble detect ---------------
//	pll.cycle = 0;
//	pll.phaseHold = False;
//	pll.polarity = 0;
//	pll.hystCnt = 0;
//	pll.phcos = 0;
//	pll.phsin = 0;
//	pll.sigPower = 0;
//	pll.agcHold = False;
//	pll.intPhase = 0;
//	pll.proPhase = 0;
//	pll.sGainIndex = GAIN_TABLE_UNITY_IX;

//	memset(sinBuf, 0, PLL_FIR_LEN*sizeof(s16));

	SetLED(PLC_RX_BUSY_LED,  0);	// Turn RX BUSY LED OFF

	return;
}






/*==========================================================================================
Function:		ProcessRxPlcMsg()

Description: 	This function processes the message in the rxUserDataArray looking
				for parity errors and extracts message contents.

Revision History:
08/12/04	HEM		New Function.
08/17/04	HEM		Added parity checking.
//==========================================================================================*/
void ProcessRxPlcMsg(void)
{
	u16			i;					// Loop counter
	u16			*upCmd;				// working pointer
	u16			uRxMsgLen;			// Message length (words)
	
	#if	USE_CRC  == True
		u16		uCRCcalc;
		u16		uCRCrec;
	#endif


	ulPlcStats[RX_CNT][plcModeSnap]++; 	// Increment total receive packet counter		

	//---- find the location of the EOP byte (there may be two) ----
	uRxMsgLen = uRxByteCount-1;
	if( rxUserDataArray[uRxMsgLen] == 0xE660 )
		uRxMsgLen--;
	if( rxUserDataArray[uRxMsgLen] == 0xE660 )
		uRxMsgLen--;
		

	//---- Compare sent CRC to calculated CRC -----------------
	#if	USE_CRC	  
		uCRCrec =    (rxUserDataArray[uRxMsgLen-1] & 0xFF00)		// the sent CRC will the the two bytes 
				  | ((rxUserDataArray[uRxMsgLen  ] & 0xFF00)>>8); 	// just ahead of the EOP byte

		uCRCcalc = CalcCRC(RX_MODE, uRxMsgLen-1);					// Calculate the CRC from the rest of the data
		if (uCRCrec == uCRCcalc)
		{
			ulPlcStats[RX_GOOD][plcModeSnap]++; // Increment good packet counter
			SetLED(PLC_RX_GOOD_LED,  1);		// Turn RX GOOD LED ON	

			// If this message is addressed to me, copy it into my command buffer 
			// and set flags to execute it in main loop
			if ( ((rxUserDataArray[0]&0xFF00) | (rxUserDataArray[1]>>8))  == uMyAddress)	
			{	
				upCmd = upCommand;
				for (i= 2; i<uRxMsgLen-1; i++)
				{
					*upCmd++ = (rxUserDataArray[i]>>8) & 0x00FF;
				} 
				uCommandActive = 1;		// Command is now active - start running command task in main loop.
			}
		}
		else  // CRC failed to match
		{
			ulPlcStats[RX_ERR_CRC][plcModeSnap]++;	// Increment CRC error counter
		}
	#endif

	uRxMsgPending = ~True;		// don't call this again until a new message arrives
	return;
}



