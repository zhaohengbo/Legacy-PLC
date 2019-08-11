//==========================================================================================
// Filename:		uart.c
//
// Description:		Functions related to the UART interface.
//
// Copyright (C) 2002 Texas Instruments Incorporated
// Texas Instruments Proprietary Information
// Use subject to terms and conditions of TI Software License Agreement
//
// Revision History:
// 05/21/02	EGO		Copied file from jervis project.
// 06/18/02 EGO		Include "timer.h".  Reaching last sub period, signals time to exit
//						HandleUART function.
// 08/06/02 EGO		Put ESTOP0 calls to help debug possible corruption.
//					Fixed comments.
// 15Feb05  Hagen	reduced size of UART out array from 100 to 16 messages
// 22Feb05	Hagen	moved InitSci() from sci.c to here
// 23Feb05	Hagen	rewrote HandleUart() to check one byte at a time
//==========================================================================================

#include "main.h"


//==========================================================================================
// Local function prototypes
//==========================================================================================


//==========================================================================================
// Local constants
//==========================================================================================
// Number of elements allowed in UARTDataOut array
#define SIZE_UART_OUT_ARRAY		(16)

// Error returned when trying to allocate past SIZE_UART_OUT_ARRAY
#define ERR_LIST_FULL			(1)

// To avoid spending too much time in TaskUART, limit the number of characters that can
// be sent to the FPGA in one pass through the task.  This limit is used as an upper
// bound for the number of char spaces available in the FIFO.
//#define MAX_SEND_CHARS			(16)


//==========================================================================================
// External Variables
//==========================================================================================
typedef struct BufferListStruct
{
	u16		uCount;			// Number of words to send at this location.
	u16*	upData;			// Location to start sending data.
} BufferListStruct;

BufferListStruct UARTDataOut[SIZE_UART_OUT_ARRAY];


//==========================================================================================
// Function:		InitializeUARTArray()
//
// Description:  	This function is used to initialize the UART, so that it comes up with
//					nothing in the buffer to send out.
//					
// Revision History:
// 05/21/02 EGO		Added uFPGAAddress field initialization.
// 06/18/02 EGO		Removed FPGA stuff.
// 07/09/02 EGO		Clean arrary for debugging.
//==========================================================================================
void InitializeUARTArray(void)
{
	u16		i;

	// Zero out the entire buffer used to send data to the UART
	for(i=0; i<SIZE_UART_OUT_ARRAY; i++)
	{
		UARTDataOut[i].uCount = 0;
		// Next line not needed.  Cleaner for debug & no harm...
		UARTDataOut[i].upData = (u16*) 0;
	}
}


//==========================================================================================
// Function:		InitSCI()
//
// Description: 	Initialize the SCI ports to a known state.
//					Currently only using SCI-A.
//
// Revision History:
// 06/18/02	EGO		Started function.
// 07/09/02 EGO		Hopefully temporary - reduced baud rate to 9600.
//					Enable interrupts (still masked in IMR).
//					Perform SCI software reset after config.
// 07/17/02 EGO		Baud rate back to 115.2K !!
//					Configure to use FIFOs.
// 08/26/02 EGO		Comment change to reflect changes in UART.c
// 02/06/03 HEM		Change baud rate to work at 120 MIPS.
//==========================================================================================
void InitSci(void)
{
	// Initialize SCI-A:

	// Communications control register
	

 	SciaRegs.SCICCR.bit.SCICHAR = 8-1;		// 8 data bits.
	SciaRegs.SCICCR.bit.ADDRIDLE_MODE = 0;	// IDLE Mode compatible with RS232.
	SciaRegs.SCICCR.bit.LOOPBKENA = 0;		// Loop Back test mode disable.
	//SciaRegs.SCICCR.bit.PARITYENA = 1;	// Parity enable.   
	SciaRegs.SCICCR.bit.PARITYENA = 0;		// Parity disable.   
	SciaRegs.SCICCR.bit.PARITY = 1;			// Even Parity.
	SciaRegs.SCICCR.bit.STOPBITS = 1-1;		// 1 Stop Bits

	// Control register 1
	SciaRegs.SCICTL1.bit.RXENA = 1;			// SCI receiver enabled
	SciaRegs.SCICTL1.bit.TXENA = 1;			// SCI transmitter enabled
	SciaRegs.SCICTL1.bit.SLEEP = 0;			// disable SCI sleep feature
	SciaRegs.SCICTL1.bit.TXWAKE = 0;		// disable Transmitter wakeup method

	SciaRegs.SCICTL1.bit.RXERRINTENA = 0;	// Receive error interrupt disable.

	// Set baud rate to 115200.
	
#if (MIPS == 120)	
	SciaRegs.SCIHBAUD = 0;   				// Baud rate (high) register
	SciaRegs.SCILBAUD = 32;   				// Baud rate (low) register
#else	// MIPS == 150
	SciaRegs.SCIHBAUD = 0;   				// Baud rate (high) register
	SciaRegs.SCILBAUD = 40;   				// Baud rate (low) register
#endif

#if	(MIPS == 100) // MIPS == 100
	SciaRegs.SCIHBAUD = 0;
//	SciaRegs.SCILBAUD = 54;
	SciaRegs.SCILBAUD = 26;
#endif

	// Control register 2.  Disable interrupts as we run in polling mode.
	SciaRegs.SCICTL2.bit.TXINTENA = 1;		// Transmit interrupt enable    
	SciaRegs.SCICTL2.bit.RXBKINTENA = 1;	// Receiver-buffer break enable

	// SCI FIFO Registers

	// FIFO transmit register

	SciaRegs.SCIFFTX.all = 0;
	// FIFO recieve register
	SciaRegs.SCIFFRX.bit.RXFFIENA = 1;
	SciaRegs.SCIFFRX.bit.RXFFIL = 1;
	// FIFO control register
	SciaRegs.SCIFFCT.all=0;
	// FIFO Priority control
	SciaRegs.SCIPRI.all=0;

	// FIFO is configured, now enable.
	SciaRegs.SCIFFTX.bit.SCIRST = 1;		// FIFO reset.
	SciaRegs.SCIFFTX.bit.SCIFFENA = 1;		// FIFO Enhancement enable

	
	SciaRegs.SCIFFRX.bit.RXFIFORESET = 1;	// Enable receive FIFO operation.
	SciaRegs.SCIFFTX.bit.TXFIFOXRESET = 1;	// Enable receive FIFO operation.


	// Initialize SCI-B:

	// SCI-B is unused currently.  
	ScibRegs.SCICTL1.bit.RXENA = 0;			// SCI-B receiver disabled
	ScibRegs.SCICTL1.bit.TXENA = 0;			// SCI-B transmitter disabled

	SciaRegs.SCICTL1.bit.SWRESET = 0;		// Software reset.
	RPTNOP(10);
	SciaRegs.SCICTL1.bit.SWRESET = 1;		// Software reset. Take out of reset.

	return;
}	


//==========================================================================================
// Function:		WriteUARTValue()
//
// Description:  	This function is used to send a single value to the UART.
//					The calling function doesn't have to worry about memory management.
//					
//					This function takes the passed value, assigns it to a variable, then
//					sends a pointer to that variable along with the destination FPGA address
//					and a count of one, to the WriteUART() function.
//
//					The "variable" used, is an element in a circular buffer.  The buffer
//					is the same length as the UARTDataOut array, so this shouldn't ever
//					have values overwritten (we should get an error from WriteUART() first).
//
// Input:			A single u16 value to be sent to the UART.
//
// Return:			SUCCESS if all went as expected, or the error code from WriteUART().
//
// Revision History:
// 05/21/02 EGO		Modified from jervis function to include FPGAAddress.
// 06/18/02 EGO		Back to a more Jervis looking routine w/ removal of FPGA.
//==========================================================================================
u16 WriteUARTValue(u16 uValue)
{
	static u16	UARTData[SIZE_UART_OUT_ARRAY];
	static u16	uIndex = 0;
	u16			uReturnValue;


	// Get the passed value and put it in the local array.
	UARTData[uIndex] = uValue;
	
	// Now use WriteUART() to add a pointer to this value to the global array that
	// taskWriteUART uses.
	uReturnValue = WriteUART(1, (u16*) &UARTData[uIndex]);		// count, pointer

	if (uReturnValue == SUCCESS)	// Value was successfully added to array
	{
		if (++uIndex == SIZE_UART_OUT_ARRAY)	// increment index into local array, check
		{										// for end of buffer.
			uIndex = 0;				// Back to start of circular buffer.
		}
	}

	if(uReturnValue != 0)
	{
	   asm ("      ESTOP0");		// Shouldn't ever get here...
	}

	return (uReturnValue);
}


//==========================================================================================
// Function:		WriteUART()
//
// Description: 	This function is used to interface between the code and the task
//					which sends data to the UART (TaskWriteUART).
//
//					This routine accepts a count, and a pointer to the start of data.  It
//					then inserts an entry into the array UARTDataOut which TaskWriteUART
//					uses to send out data.
//					Note that it will store the count and pointer, not buffer the actual
//					data, so the calling function must keep the variable around (static or
//					global), or else use the function WriteUARTValue().
//
// Input:			u16  uCount;		Number of words to transfer.
//					u16* upData;		Pointer to start of data.
//
// Return:  		SUCCESS  		Added to list. (not necesarily sent yet)
//					ERR_LIST_FULL   No buffer space.
//
// Revision History:
// 05/21/02 EGO		Modified from Jervis to include FPGA Address.
// 05/29/02 EGO		Added element uPulseCommand to UARTDataOut structure.
// 06/18/02 EGO		Back to Jervis style w/o FPGA
// 07/09/02 EGO		More explicit casting.
// 16Feb05  Hagen	changed uIndex check
//==========================================================================================
u16 WriteUART(u16 uCount, u16* upData)
{
	static u16	uIndex = 0;			// Index into UARTDatatOut array for passed data.
	u16			uMaxIndex;			// Where we'll stop looking for an available space.
	u16			uAdded = 0;			// Set when data has been added to the array.
	u16			uReturnValue;		// Function return value


	uMaxIndex = uIndex-1;			// We'll look through the entire array (once).
	if (uMaxIndex == 65535)
	{
		uMaxIndex = SIZE_UART_OUT_ARRAY - 1;
	}

	// I know it's negative, but assume we'll fail, and change later when we succede.
	uReturnValue = ERR_LIST_FULL;

	// Keep looking for an available entry until we find one, or go through all of them.
	while ( (uAdded == 0) && (uIndex != uMaxIndex) )
	{
		if (UARTDataOut[uIndex].uCount == 0)		// If count==0, this entry is free.
		{
			UARTDataOut[uIndex].upData = upData;	// Add data pointer
			UARTDataOut[uIndex++].uCount = uCount;	// Add count
			uAdded = 1;								// Flag signalling done
			uReturnValue = SUCCESS;					// Be positive.
		}
		else	// That entry was being used, try the next one.
		{
			uIndex++;								// increment index
			if (uIndex >= SIZE_UART_OUT_ARRAY)		// If we go past the end ...
			{
				uIndex = 0;							// ... start over at the beginning
			}
		}
	}

	if(uReturnValue != 0)
	{
	   asm ("      ESTOP0");		// Shouldn't ever get here...
	}

	return (uReturnValue);
}


/*==========================================================================================
Function:		HandleUART()

Description: 	This function is used to send/receive data to the UART.
				It is called every N ADC sample periods.  It is assumed that the 
				rate at which it is called is fast enough to keep the TX buffer full and
				to keep ahead of the RX buffer.  
				
				The serial interface bit rate is set to 115.2kb/s or 11520 bytes/sec
				
				The PLC ADC sample rate is 115 ksamples/s and this routine is called once 
				every ADCINT_COUNT_MAX ADC intrupt times.

				Because the defined command interface only allows one command to be
				issued at a time, the flow of UART data consists of a command coming
				in to the DSP, and then a response going out.  A second command can not
				be sent until the reply from the first is received.
					
 Revision History:
 05/29/02 EGO		Started with Jervis function TaskWriteUART.
 10/15/02 HEM		Changed from Timer2 to Timer0.  Changed reg name to match data sheet.
 02/17/05 Hagen		Rewrote the routine so that we only do one receive byte or two transmit 
 					bytes at a time.  
==========================================================================================*/
#define RTS_ENABLE  	(0)
#define RTS_DISABLE 	(1)
#define RECEIVE_TIMEOUT	(11500)			// 1/2 second w/o character = flush.
#define	ALMOST_FULL		12				// Choose hold-off level for buffer.  Must be <= 15

void HandleUART(void)
{
	static u16 	uIndex = 0;				// Index into UARTDatatOut.
	static u16 	uBytesReceived = 0;		// Number of bytes received for the command being formed.
	static u16 	uParmNumber = 0;		// Which parm number we're constructing
	static u16 	uSampleCounter = 0;		// No action counter for resetting command buffer


	//---- check for serious screw-ups -----------------------
	if(uIndex >= SIZE_UART_OUT_ARRAY)
	{
	   asm ("      ESTOP0");		// Shouldn't ever get here...most likely uIndex overwritten somewhere
	}

	if(SciaRegs.SCIRXST.bit.RXERROR == 1)	// Receiver Error.
	{
		InitSci();		// Reset port.
	}
	
	/*----------------------------------------------------------------
	This section is used to prevent the DSP and Matlab from getting out of sync.
	If the DSP is in the middle of receiving a command, and encounters a pause of more
	than a half second, it will assume it missed part of the command and reset itself for
	another command.  Matlab can use this to reset this HandleUART() function by simply
	sending one byte, then waiting for a second to pass.
	-----------------------------------------------------------------*/
	if (uSampleCounter != 0)	// Will equal zero until a character is received.
	{
		if (uSampleCounter++ >= RECEIVE_TIMEOUT)	// A1/2 second has passed with no chars received.
		{
			#ifdef DIAG_TRACE
				SaveTraceF(0xEEEE);		 
				SaveTraceF(uSampleCounter);		 
			#endif

			uBytesReceived = 0;		// Reset for next command.
			uParmNumber = 0;		// Reset for next command.
			uSampleCounter = 0;		// Deactivate the timeout counter.
		}
	}


	/*----------------------------------------------------------------
	Transmit RS-232 bytes by filling the UART transmit buffer.
	
	Each time through the routine, add two more bytes to the UART FIFO until 
	it is almost full (12 out of 16 bytes).  Then keep it full until the data is sent.
	-----------------------------------------------------------------*/
	if( UARTDataOut[uIndex].uCount > 0)
	{
		if( SciaRegs.SCIFFTX.bit.TXFFST < ALMOST_FULL )
		{
			// fill the FIFO by writing to SciaRegs.SCITXBUF   
			SciaRegs.SCITXBUF = (UARTDataOut[uIndex].upData[0]) >> 8;
			SciaRegs.SCITXBUF = UARTDataOut[uIndex].upData[0];
			UARTDataOut[uIndex].upData = (u16*) &(UARTDataOut[uIndex].upData[1]);

			UARTDataOut[uIndex].uCount--;
			if( UARTDataOut[uIndex].uCount <= 0 )
			{
				uIndex++;			// Point to next element in the data array
				if( uIndex == SIZE_UART_OUT_ARRAY ) 
					uIndex = 0;
			}

		}
	}

	#ifdef DIAG_TRACE
		if( uloopCnt > 0 )
		{
			SaveTraceF(0xCC00 + UARTDataOut[uIndex].uCount);		 
			SaveTraceF(SciaRegs.SCIFFTX.bit.TXFFST);		 
		}
	#endif


	/*----------------------------------------------------------------
	Receive RS-232 bytes by filling the UART receive buffer.
	
	Each time through the routine, pull a byte out of the buffer and put it in 
	the command buffer.  Since sample rate is faster than the baud rate we allways 
	stay caught up.
	-----------------------------------------------------------------*/

	// Enable host to send data if there is room in rx buffer
	if (SciaRegs.SCIFFRX.bit.RXFFST < ALMOST_FULL)
		UART_RTS = RTS_ENABLE;				// Enable host to send data.
	else
		UART_RTS = RTS_DISABLE;				// Stop receiving data.
	

	if( SciaRegs.SCIFFRX.bit.RXFFST >= 1 )
	{
		uSampleCounter = 1;		// Reset/start the "count without characters".

		if (IsEven(uBytesReceived))		// MSB
		{
			upSerialCommand[uParmNumber] = SciaRegs.SCIRXBUF.bit.RXDT << 8;
			uBytesReceived++;
		}
		else	// It's an odd byte, so add it to the previously received even byte
		{		//	to form a command parm word.
			upSerialCommand[uParmNumber++] += SciaRegs.SCIRXBUF.bit.RXDT;
			uBytesReceived++;
		}

		if (uParmNumber == COMMAND_PARMS)		// We've read in an entire command.
		{
			uBytesReceived = 0;		// Reset for next command.
			uParmNumber = 0;		// Reset for next command.

			uCommandActive = 1;		// Command is now active - start running command
									//		task in main loop.
			uCommandPending = 1;	// Tell command task to process new command.
			uSampleCounter = 0;		// reset timeout check
		}
	}
	
	#ifdef DIAG_TRACE
		if( uloopCnt > 0 )
		{
			SaveTraceF( 0xDD00 + uBytesReceived );		 
			SaveTraceF( SciaRegs.SCIFFRX.bit.RXFIFST );
		}
	#endif

	return;
}

