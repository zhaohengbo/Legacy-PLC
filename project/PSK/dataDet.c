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

#define ETA0				(9362-40)	// nominal sample to sample interval in pll.phase at Fc
#define	ETA_RNG_LO			(-300)		// limit integrating phase error state to this range
#define	ETA_RNG_HI			(+300)
#define CYCLES_BIT			3			// number of cycles of downsampled carrier per bit

#define PLL_K_PRO			101			// PLL feedback gains
#define PLL_K_PRO_SCALE		11
#define PLL_K_INT			80	
#define PLL_K_INT_SCALE		14		

#define FIND_WORDSYNC_TO	24			// timeout after N bit times
#define FIND_EOP_TO			(8*128)		// timeout at 128 bytes

#define PLL_VHYST_THRS		1024   		// vertical bits
#define PLL_THYST_THRS		4      		// time samples
#define PLL_HOLD_THRS		110			// sample detBit signal above this point in pll.bitPhase

#define AGC_THRS			(1500)		// control agcSignal to this amplitude 
#define AGC_ENABLE			True		// turn on Automatic Gain Control


#ifdef DSP_COMPILE
	// Functions that will be run from RAM need to be assigned to 
	// a different section.  This section will then be mapped using
	// the linker cmd file.
	#ifdef __cplusplus			// "C++"
		#pragma CODE_SECTION("ramfuncs"); 
	#else						// "C"
		#pragma CODE_SECTION(runPLL, "ramfuncs");
		#pragma CODE_SECTION(receive, "ramfuncs");
	#endif
#endif


#ifdef MEX_COMPILE
#define SINTABLE_LEN		64
#define	PLL_FIR_LEN			(2*7)

#define BITSYNC_PATTERN		0xAAAA
#define WORDSYNC_PATTERN	0x567B
#define WORDSYNC_PAT_NEG	(0xFFFF ^ WORDSYNC_PATTERN)
#define EOP_PATTERN			0x0733

#define AGC_TABLE_LEN		32
#define AGC_SCALE			5
#define GAIN_INDEX_THRS		200
#define GAIN_TABLE_SCALE	4

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
Function:		runPLL()

Description: 	This function uses a look-up table technique to generate a sine wave 
				(both cos and sin) that is multiplyed with received ADC samples.  This product 
				is summed over one cycle of the sine wave to calculate a phase between the 
				generated sine wave and the received signal. 

				When the two signals are phase locked the cosine sum represents the 0 degree
				and 180 degree phase data and the sine sum represents the phase error.  The 
				sine sum phase error is used to adjust the frequency of the calculated sine 
				wave.  In so doing, a digital phase lock loop function is created.
	
Global vars:
	pllControl	pll; 	// PLL properties used in other functions are collected in this structure
  

Revision History:
01Feb05	Hagen	Created function
25Feb05	Hagen	added more comments
07Mar05	Hagen	changed gainIndex inc from -96  to -48
==========================================================================================*/
s16 runPLL(s16 signal )
{
	u16				prevPhase = 0;			
	s16				pherr;
	u16				tabIndex;
	s16				bcos, bsin;
	s32				signal32;
	s16				agcSignal;
	static s16		agcGain	= (1<<AGC_SCALE);
	
	//---- apply AGC gain and do coarse gain adjust -------------
	#if AGC_ENABLE == True
		signal32 = ((s32)signal * (s32)agcGain) >>AGC_SCALE;
		if (pll.agcHold == False)
		{
			while(signal32 != (s16)signal32)	
			{	// If 32-bit version does not match 16-bit version, reduce agcGain until it does
				pll.sGainIndex -= 48;
				if( pll.sGainIndex <= 0)
					pll.sGainIndex = 0;
				agcGain = gainTable[pll.sGainIndex>>GAIN_TABLE_SCALE];
				
				signal32 = ((s32)signal * (s32)agcGain) >>AGC_SCALE;
			}
		}
		signal = (s16)signal32;
	#endif	
			
    //---- calculate sine generator (VCO) phase -----------------------
	prevPhase = pll.phase;
    	pll.phase = pll.phase + ETA0 + pll.eta;
	if( prevPhase >= pll.phase )		// phase has wrapped around
	{
		pll.cycle++;					// there are three cycles of 16.5 kHz per bit
		if( pll.cycle >= 3 )
			pll.cycle = 0;
	}
    tabIndex = pll.phase >> 10;			// sine table index

	//---- calc phase within a bit window ---------------------------------
	pll.bitPhase = (pll.cycle << 6) + tabIndex;	 // this is used externally
												 // to set pll.phaseHold

	//---- sine and cosine VFO output -------------
	bcos = sinTable[tabIndex];
    bsin = sinTable[tabIndex+(SINTABLE_LEN/4)];

	//---------------------------------------------
	//  To calculate phase error between VFO and the signal,
	//  multiply the sine and cosine with the signal ADC value and sum
	//  over one period.  This is done by putting the product into a 
	//  7 element circular buffer, then subtracting the oldest value and adding
	//  the newest value to the cosine sum and to the sine sum.
	pll.phcos -= sinBuf[pll.firCnt];
	sinBuf[pll.firCnt] = (s16)( ( (s32)(bcos) * (s32)(signal) ) >> 16 );
	pll.phcos += sinBuf[pll.firCnt];
	#if AGC_ENABLE == True
		agcSignal  = abs(sinBuf[pll.firCnt++]);
	#else
		pll.firCnt++;
	#endif

	pll.phsin -= sinBuf[pll.firCnt];
	sinBuf[pll.firCnt] = (s16)( ( (s32)(bsin) * (s32)(signal) ) >> 16 );
	pll.phsin += sinBuf[pll.firCnt++];

	if( pll.firCnt >= PLL_FIR_LEN )
		pll.firCnt = 0;

	//---- calculate phase error ---------------------------------
    if( pll.phcos < 0 )			// When the DPLL is locked to the signal the cosine sum will
		pherr = -pll.phsin;		// be a large positive or negative value and the sine sum
	else						// will be near zero.  So the sine represents the phase error.
		pherr = pll.phsin;		// The polarity of which depends on if we are phase locking to 
								// 0 degrees (1-bit) or 180 degrees (0-bit).
						
	if( ~pll.phaseHold )
	{
		//---- calculate control effort -----------------
	    pll.proPhase  = ((s32)pherr*PLL_K_PRO) >> PLL_K_PRO_SCALE;	// proportional feedback
		pll.intPhase += ((s32)pherr*PLL_K_INT) >> PLL_K_INT_SCALE;	// integral feedback
		pll.intPhase  = Saturate(pll.intPhase, ETA_RNG_LO, ETA_RNG_HI);
		pll.eta = pll.intPhase + pll.proPhase;
	}	// end pll hold


	#if AGC_ENABLE == True
		//---- adjust signal amplitude -------------
		if( ~pll.agcHold )
		{
			if( agcSignal < AGC_THRS )
			{
				pll.sGainIndex++;
				if( pll.sGainIndex > ((GAIN_TABLE_LEN-1)<<GAIN_TABLE_SCALE) )
					pll.sGainIndex = ((GAIN_TABLE_LEN-1)<<GAIN_TABLE_SCALE);
			}
			else   
			{
				pll.sGainIndex -= 2;
				if( pll.sGainIndex < 0 )
					pll.sGainIndex = 0;
	
			}	 //end gain adjust
			agcGain = gainTable[pll.sGainIndex>>GAIN_TABLE_SCALE];
		}	// end agc hold
	#endif


	//---- diagnostics ---------------------
	#if TRACE_BUF_LEN > 0
		#ifdef DSP_COMPILE
			SaveTrace(signal);		 
			//SaveTrace(agcSignal);		 
		    SaveTrace(pll.phcos);   
		    //SaveTrace(pherr);		
			//SaveTrace(pll.bitPhase);	
			SaveTrace(pll.sGainIndex); 
			//SaveTrace(uRxMode);	   
		#endif
		#ifdef MEX_COMPILE
			SaveTrace( pll.phcos );
			SaveTrace( pherr );
			SaveTrace( signal );
			SaveTrace( pll.sGainIndex );
			//SaveTrace( pll.intPhase );
			//SaveTrace( pll.proPhase );
			//SaveTrace( pll.eta );
			SaveTrace(agcSignal);
		#endif
	#endif

	return pll.phcos;
}


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
void receive(s16 ADCsample)
{
	static u16	bitNum;					// count the bits in a byte
	static u16	detData  = 0;			// receive data word
	static u16  detBit = 0;				// squared-up version of phase data
	static s16  bitSample = False;		// flag used to sample detBit in FIND_DATA
	static u16 	uEOP_holdOffCnt = 0;	// time to wait before transmitting

	s16			phcos;					// phase data out of digital PLL
	u16			bitTransition = False;	// flag used to sample detBit in FIND_BITSYNC
	s16			diagSample = 0;			// flag used to generate trace data 
	
	
	//---- run the digital PLL for this sample -------------
	phcos = runPLL( ADCsample );


	//---- apply time and voltage hysteresis to the phase data: phcos ------
	if( detBit == 1 )							// detBit is pos
	{
		if( phcos < (-PLL_VHYST_THRS) )			// phcos is neg
		{
			pll.hystCnt++;
			if( pll.hystCnt > PLL_THYST_THRS )
			{
				pll.hystCnt = 0;
				detBit  = 0;					// transistion pos-neg
				bitTransition = True;
				if( pll.bitPhase < 160 )
					pll.cycle = 0;
			}
		}
	}
	else										// detBit is neg
	{
		if( phcos > PLL_VHYST_THRS )			// phcos is pos
		{
			pll.hystCnt++;
			if( pll.hystCnt > PLL_THYST_THRS )
			{
				pll.hystCnt = 0;
				detBit  = 1;					// transistion neg-pos
				bitTransition = True;
				if( pll.bitPhase < 160 )
					pll.cycle = 0;
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
		uRxModeCount++;
		if( bitTransition )
		{
			//---- reset if too long or too short between bit transitions -----------
			if( (uRxModeCount > (21+11)) || (uRxModeCount < (21-11)) )
				detData = 0;
			else
				detData = (detData << 1) | detBit;
						
			uRxModeCount = 0;
			diagSample = 1;
			pll.phaseHold = False;	// make sure PLL is running
		
			//---- compare to preamble pattern -------------
   			if( detData == BITSYNC_PATTERN )
			{
				uRxMode = FIND_WORDSYNC;
				uRxModeCount = 0;
				bitSample = True;
				pll.agcHold = True;
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
		if( pll.bitPhase > PLL_HOLD_THRS )
		{
			pll.phaseHold = True;	// hold PLL 
			if( bitSample )
			{
				bitSample = False;
				detData = (detData << 1) | detBit;
				diagSample = 1;

				//---- look for WordSync ------------
				if( detData == WORDSYNC_PATTERN )
				{
					uRxMode = FIND_DATA;
					pll.polarity = 0;
				}
				if( detData == WORDSYNC_PAT_NEG )
				{
					uRxMode = FIND_DATA;
					pll.polarity = 1;
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
		}			// if (bitPhase )
		else		// bitPhase outside of PLL-Hold time 
		{
			pll.phaseHold = False;	// run PLL
			bitSample = True;		// enable next bit sample
		}

		break;

	//---- look for data --------------------------------------
	case FIND_DATA:
	case FIND_EOP:
		if( pll.bitPhase > PLL_HOLD_THRS )
		{
			pll.phaseHold = True;	// hold PLL 
			if( bitSample )
			{
				bitSample = False;
				detData = (detData << 1) | (detBit^pll.polarity);
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
					//rxUserDataArray[uRxByteCount++] = detData << 5;  // store data in upper byte
					
					rxUserDataArray[uRxByteCount] = detData << 5;  // store data in upper byte					
					
					if( (uRxMode == FIND_DATA) && ( rxUserDataArray[uRxByteCount] != uTxPrecodeTable[detData>>3] ) )
					{
						ulPlcStats[RX_ERR_PARITY][plcModeSnap]++; 	// Count EOP patterns found 
					}
					
					uRxByteCount++;
					detData = 0;					// clear out detData for next byte
					bitNum = CODEWORD_LEN;			// this leaves parity in low byte
					//---this is debug overhead! -------------
					//if( uRxByteCount >= 3 )
					//{
					//	NOP;  
					//	NOP;	// spot to set a breakpoint
					//} 


				}

			}		// if( bitSample )
		}			// if (bitPhase )
		else		// bitPhase outside of PLL-Hold time 
		{
			pll.phaseHold = False;	// run PLL
			bitSample = True;		// enable next bit sample
		}

		break;

	}	// switch block

	//---- diagnostics ---------------------
	#if TRACE_BUF_LEN > 0
		#ifdef DSP_COMPILE
			SaveTrace( (uRxMode<<2) + (diagSample<<1) + detBit );
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
	
	//---- set up for preamble detect ---------------
	pll.cycle = 0;
	pll.phaseHold = False;
	pll.polarity = 0;
	pll.hystCnt = 0;
	pll.phcos = 0;
	pll.phsin = 0;
	pll.sigPower = 0;
	pll.agcHold = False;
	pll.intPhase = 0;
	pll.proPhase = 0;
	pll.sGainIndex = GAIN_TABLE_UNITY_IX;

	memset(sinBuf, 0, PLL_FIR_LEN*sizeof(s16));

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



