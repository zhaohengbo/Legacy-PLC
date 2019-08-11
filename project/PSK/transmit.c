//==========================================================================================
// Filename:		transmit.c
//
// Description:		Power-Line Communicationsfile.
//					Contains functions to handle communications over power-line.
//
// Copyright (C) 2004 Texas Instruments Incorporated
// Texas Instruments Proprietary Information
// Use subject to terms and conditions of TI Software License Agreement
//
// Revision History:
// 04/15/04	HEM		New file.
//==========================================================================================

#include "main.h"         
#include "transmit.h"         
#include <string.h>					// contains memset()


void HandlePLC(void);
void SetPlcMode (u16 mode);
void ExtractNextTxBit(void);
void ExtractNextRxBit(void);


// Functions that will be run from RAM need to be assigned to 
// a different section.  This section will then be mapped using
// the linker cmd file.
#ifdef __cplusplus			// "C++"
	#pragma CODE_SECTION("ramfuncs"); 
#else						// "C"
	//#pragma CODE_SECTION(HammingDistance, "ramfuncs");
	//#pragma CODE_SECTION(SetRxMode, "ramfuncs");

	//#pragma CODE_SECTION(T1PINT_ISR, "ramfuncs");
	#pragma CODE_SECTION(ADCINT_ISR, "ramfuncs");

	//#pragma CODE_SECTION(FilterCorrRx, "ramfuncs");
#endif



/*----------------------------------------------------------------
 	Global vars used in this module
----------------------------------------------------------------*/

u16	uTxBitNum=0;		// Bit number within transmit message that is being transmitted next
u16 uTxMsgLen;			// Total number of bits in transmit message, including header, trailer, start/stop and parity.
u16	uCollisionCntr = 0;	// Flag to indicate collision has occurred during this transmission

q16	qPolarity= 0;		// Polarity of most recently received bit




//==========================================================================================
// Function:		GenerateFakePLCMessage()
//
// Description: 	Generate a fake message for the PLC to send.
//					This function is required only for development and testing.  
//					In a real system, it would be replaced be a function that generates
//					real messages from a control process or from an external computer interface.
//
// Revision History:
// 09/02/04	HEM		New function
// 11/17/04	HEM		Reduce longest fake message from 32 to 16 bytes.
//==========================================================================================
u16 GenerateFakePLCMessage(u16	uSeed)
{
	u16	i;
	u16	j;
	u16 k;
	u16 uMsgLen;

	k = uSeed;	// Starting value
	j = Saturate(uSeed >> 8, 1, 255); 	// Increment size
	uMsgLen = Saturate( ((uSeed >> 2) & 0x0F), 6, MAX_TX_MSG_LEN );	// Generate a message length between 6 and 16 bytes long			 
	for (i=0; i<uMsgLen; i++)
	{	
		txUserDataArray[i]= (k + (j * (i-0))) & 0xff; //!!!FAKE!!! Generate fake message string to send
	}

	for (i=uMsgLen; i<MAX_TX_MSG_LEN; i++)
	{	
		txUserDataArray[i]= 0; 	// Zero the rest of the tx array
	}

	return(uMsgLen);
}


//==========================================================================================
// Function:		GenerateFloodPLCMessage()
//
// Description: 	Generate a message to "flood" the power line.  
//					This is used to measure bit error rate (BER)
//					format of packet to send:
//					   byte	description
//						0	destination address high byte
//						1	destination address low byte
//						2	echo command (0020h)
//						3	my address high byte
//						4	my address low byte
//						5	ack command (0021h)
//					30-31	CRC	
//
// Revision History:
// 09/02/04	HEM		New function
// 11/17/04	HEM		Reduce longest fake message from 32 to 16 bytes.
//==========================================================================================
u16 GenerateFloodPLCMessage(u16	uSeed)
{
	u16	i = 0;

	//---- Set up Echo command -----
	txUserDataArray[i++] = uDestAddress>>8;			// slave address
	txUserDataArray[i++] = uDestAddress& 0x00FF;	// set in CmdPLCEchoSet();
	txUserDataArray[i++] = CMD_ECHO_CMD;
	txUserDataArray[i++] = uMyAddress>>8;			// master address
	txUserDataArray[i++] = uMyAddress& 0x00FF;
	txUserDataArray[i++] = CMD_ECHO_ACK;
	txUserDataArray[i++] = SUCCESS;				// return code
	
	uSeed &= 0x00FF;
	for (; i<MAX_TX_MSG_LEN; i++)
	{	
		txUserDataArray[i]= uSeed++ & 0xFF;	//Fill buffer with some non-zero data
	}
		
	uTxMsgPending = True;	// Set flag to tell main loop to start sending message when traffic permits
	uCommandActive = 0;		// Command is done.  Allow TaskCommand to finish up.
	ulBerStats[BER_TX_COUNT]++;				// Add more here!!!
	uCmd_EchoAck = False;	// toggle the var while sending BER packet

	return(MAX_TX_MSG_LEN);
}



//==========================================================================================
// Function:		SetPWMPolarity()
//
// Description: 	The function sets the polarity of the PWM output.
//
// Revision History:
// 04/28/04	HEM		New Function.
//==========================================================================================
void SetPWMPolarity(s16 sPolarity)
{

#define	AMP_SUM		// Analog amplifier combines sum of two PWM outputs to generate TX waveform
//#define AMP_DIFF	// Analog amplifier uses difference of two PWM outputs to generate TX waveform


#if defined(AMP_DIFF)
	if(sPolarity == 1)
	{	// Transmit with positive polarity
		EvaRegs.ACTRA.all = (EvaRegs.ACTRA.all & ~0x0033) | 0x0012; 	// PWM1=Active Hi, PWM3=Active Lo
	}
	else if (sPolarity == 0)
	{	// Transmit with negative polarity
		EvaRegs.ACTRA.all = (EvaRegs.ACTRA.all & ~0x0033) | 0x0021; 	// PWM1=Active Lo, PWM3=Active Hi
	}
	else	// sPolarity == -1
	{	// Hold both PWM outputs low
		EvaRegs.ACTRA.all = (EvaRegs.ACTRA.all & ~0x0033) | 0x0000; 	// PWM1 & PWM3= Hold Low
	}
#elif defined(AMP_SUM)
	if(sPolarity == 1)
	{	// Transmit with positive polarity
		EvaRegs.ACTRA.all = (EvaRegs.ACTRA.all & ~0x0033) | 0x0022; 	// PWM1=Active Hi, PWM3=Active Hi
	}
	else if (sPolarity == 0)
	{	// Transmit with negative polarity
		EvaRegs.ACTRA.all = (EvaRegs.ACTRA.all & ~0x0033) | 0x0011; 	// PWM1=Active Lo, PWM3=Active Lo
	}
	else	// sPolarity == -1
	{	// Hold both PWM outputs low
		EvaRegs.ACTRA.all = (EvaRegs.ACTRA.all & ~0x0033) | 0x0000; 	// PWM1 & PWM3= Hold Low
	}
	
#endif

}



//==========================================================================================
// Function:		SetPlcMode()
//
// Description: 	The function configure the Power-Line Communication mode and sets the 
//					proper variables and registers to switch between transmit and receive.
//
// Revision History:
// 04/15/04	HEM		New Function.
// 07/15/04	HEM		Added transmitter bias setting.
//==========================================================================================
void SetPlcMode (u16 mode)
{
	plcMode = mode;

	if (plcMode == TX_MODE)
	{
		SetPWMPolarity(-1);			// Turn off the transmit PWM
		TX_BIAS_EN = 1;				// Turn transmitter bias ON
		
		// Configure for Transmit Mode
		//!!! *** Add code here ***
		ExtractNextTxBit();
	}
	else //(plcMode == RX_MODE)
	{
		// Configure for Receive Mode
		//!!! *** Add code here ***
		TX_BIAS_EN = 0;				// Turn transmitter bias OFF
		SetPWMPolarity(-1);			// Turn off the transmit PWM
		#if (RECEIVE_OWN_XMIT != True)	// Re-arm the sensors, unless we are listening to our own TX, 
			ArmAllSensors();			// in which case they will be armed in ADCINT_ISR().
		#endif
	}
	
	return;
}

 
const u16	uHeader [(HEADER_LEN+15)/16] = {0xAAAA, 0xAACF, 0x6000};
const u16	uTrailer [(TRAILER_LEN+15)/16] = {0xE67C, 0xCC00};

//==========================================================================================
// Function:		FillTxBuffer()
//
// Description: 	The function fills the transmit buffer using the proper header, user data 
//					(with parity and start/stop bits), and trailer.
//
// Revision History:
// 05/04/04	HEM		New Function.
//==========================================================================================
void FillTxBuffer(u16 uUserTxMsgLen)
{
	u16	i;
	q16	j;
	u16	uBitNum;
	u16	uWord;

#if	USE_CRC  == True
	u16	uCRC;
	u16 uCRCByte;
#endif

	// Copy the header into the transmit data array
	for (i=0; i<((HEADER_LEN+15)/16); i++)
	{
		txDataArray[i] = uHeader[i];
	}

	uBitNum = HEADER_LEN;
	
	// Copy the user transmit data into the transmit data array, with start/stop and parity bits
	for (i=0; i<uUserTxMsgLen; i++)
	{
		uWord = uBitNum / 16;
		j = (uBitNum & 15);
		
		txDataArray[uWord] |= uTxPrecodeTable[txUserDataArray[i]] >> j;		// Top of precoded data word into outgoing data word
		if (j >= (16-11))
		{
			txDataArray[++uWord]  = uTxPrecodeTable[txUserDataArray[i]] << (16-j);	// Bottom of precoded data word into top of next outgoing data word		
		}
		uBitNum += 11;
	}

	// Calculate the CRC and copy it into the transmit data array, with start/stop and parity bits
	#if	USE_CRC  == True
		uCRC = CalcCRC(TX_MODE, uUserTxMsgLen);
	
		for (i=0; i<2; i++)
		{	
			uWord = uBitNum / 16;
			j = (uBitNum & 15);
			
			uCRCByte = (uCRC>>(8-(8*i))) & 0x00FF;	// Select proper byte from CRC word
			txDataArray[uWord] |= uTxPrecodeTable[uCRCByte] >> j;		// Top of precoded data word into outgoing data word
			if (j >= (16-11))
			{
				txDataArray[++uWord]  = uTxPrecodeTable[uCRCByte] << (16-j);	// Bottom of precoded data word into top of next outgoing data word		
			}
			uBitNum += 11;
		}
	#endif


	// Copy the trailer into the transmit data array
	uWord = uBitNum / 16;
	j = (uBitNum & 15);
	if (j==0)
	{
		for (i=0; i<(TRAILER_LEN+15)/16; i++)
		{
			txDataArray[uWord++] = uTrailer[i];
		}
	}
	else
	{
		for (i=0; i<(TRAILER_LEN+15)/16; i++)
		{
 			txDataArray[  uWord] |= uTrailer[i] >> j;		// Top of precoded data word into outgoing data word
			txDataArray[++uWord]  = uTrailer[i] << (16-j);	// Bottom of precoded data word into top of next outgoing data word		
		}
	}
	uBitNum += TRAILER_LEN;

	
	// Set pointer to beginning of array
	uTxMsgLen = uBitNum;
	SetPlcMode(TX_MODE);			// Switch to transmit mode	
	uTxBitNum  = 0;
	ulPlcStats[TX_CNT][TX_MODE]++; 	// Increment transmit packet counter
	
	return;
}

 
//==========================================================================================
// Function:		ExtractNextTxBit()
//
// Description: 	This function extracts the next bit from the transmit message bit string. 
//
// Revision History:
// 04/15/04	HEM		New Function.
//==========================================================================================
void ExtractNextTxBit(void)
{
	u16	uBit;
	u16	uWord;
	
	uWord = uTxBitNum / 16;
	uBit = uTxBitNum - uWord*16;
	txBitNext = (txDataArray[uWord] >> (15-uBit) ) & 1;
	uTxBitNum++;
	
	txBitPending = 1;	
	
	return;
}



//#define	TX_BIT_COUNT	24		// Carrier periods if using TX interupt
#define	TX_BIT_COUNT		21		// ADC samples per bit == tx bit timing 

//==========================================================================================
// Function:		T1PINT_ISR()
//
// Description: 	This ISR services the Event Manager Timer 1 Period Interrupt.
//					During transmit, every 24 periods the polarity of the transmit waveform
//					is adjusted.
//					During receive, a sample is copied from the A/D converter into the 
//					receive buffer.  Every 25 periods, a flag is set to alert the main program
//					that the buffer is ready to be decoded. 
//
// Revision History:
// 04/15/04	HEM		New Function.
//==========================================================================================
interrupt void T1PINT_ISR(void)    // EV-A
{
	SetXF();				// Debug: Turn on XF to show activity on external scope	
	//SetLED(PLC_TX_LED, 1);		//!!!DEBUG!!! Turn on TX LED	
/*
	if (plcMode == TX_MODE)
	{
		if (++T1PIntCount >= TX_BIT_COUNT)
		{
			T1PIntCount	= 0;
			txBitPending = 0;			// Set flag so main program knows that latest transmit bit has started sending
			SetPWMPolarity(txBitNext);	// Set polarity of PWM to match next bit transmitted
			SetLED(PLC_TX_LED, txBitNext);	// Echo the transmitted bits to the TX_LED

			if (uTxBitNum > uTxMsgLen)	// continue until end of transmit message
			{
				SetPlcMode(RX_MODE);	// Switch to receive mode
				NOP;			
			}			
			else
			{
				ExtractNextTxBit();		// Extract next Tx bit now so ISR can send it quickly when needed
			}
		}
	}
	else
	{
		SetLED(PLC_TX_LED, 0);		//!!!DEBUG!!! Turn off TX LED
	}	

	//SetLED(PLC_TX_LED, 0);		//!!!DEBUG!!! Turn off TX LED
*/
	EvaRegs.EVAIFRA.bit.T1PINT = 1;			// Clear the EVA T1PINT interrupt flag
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP2;	// Enable PIE interrupts	
	EINT;   								// Re-Enable Global interrupt INTM
	return;
}


//==========================================================================================
// Function:		ADCINT_ISR()
//
// Description: 	This ISR services the A/D converter's conversion complete interrupt.
//					The most recently measured samples are filtered and the A/D is 
//					armed to be triggered by an Event Manager timer event.
//
// Revision History:
// 04/15/04	HEM		New Function.
//==========================================================================================
interrupt void  ADCINT_ISR(void)     // ADC
{
	static s16		ADCsample[ADCINT_COUNT_MAX];
	//u16				n;
	
	SetXF();				// Debug: Turn on XF to show activity on external scope
	
	//========= TRANSMIT =====================================	
	if (plcMode == TX_MODE)
	{
		if (++T1PIntCount >= TX_BIT_COUNT)
		{
			T1PIntCount	= 0;
			txBitPending = 0;			// Set flag so main program knows that latest transmit bit has started sending
			SetPWMPolarity(txBitNext);	// Set polarity of PWM to match next bit transmitted
			SetLED(PLC_TX_LED, txBitNext);	// Echo the transmitted bits to the TX_LED

			if (uTxBitNum > uTxMsgLen)	// continue until end of transmit message
			{
				SetPlcMode(RX_MODE);	// Switch to receive mode
				NOP;			
			}			
			else
			{
				ExtractNextTxBit();		// Extract next Tx bit now so ISR can send it quickly when needed
			}
		}
	}
	else
	{
		SetLED(PLC_TX_LED, 0);	
	}	


	//========= RECEIVE =====================================	
	if ((plcMode == RX_MODE) || (RECEIVE_OWN_XMIT == True))
	{
		ADCsample[0] = SmoothADCResults();	// Combine the results from A/D Converter, 
											// scale them, and convert them to signed values	
		ArmAllSensors();					// Re-arm the sensors for the next reading
		
		ADCIntCount++;
		if( ADCIntCount >= ADCINT_COUNT_MAX )
			ADCIntCount = 0;
			
		receive(ADCsample[0]);

		/*
		//---- get the ADC sample ------------------------------ 
		ADCsample[ADCIntCount] = SmoothADCResults();	// Combine the results from A/D Converter, 
														// scale them, and convert them to signed values	
		ArmAllSensors();				// Re-arm the sensors for the next reading

		ADCIntCount++;
		if( ADCIntCount >= ADCINT_COUNT_MAX )
		{
			ADCIntCount = 0;
			for( n = 0; n < ADCINT_COUNT_MAX; n++ )
			{
				receive(ADCsample[n]);
			}
		}
		*/
		
	}
	
	uADCIntFlag = 1;						// Set flag to tell MainLoop() that ADC Interrupt occurred
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;	// Enable PIE interrupts	
	EINT;   								// Re-Enable Global interrupt INTM
	return;
}


