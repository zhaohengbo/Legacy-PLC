//==========================================================================================
// Filename:		command.c
//
// Description:		Functions related to the recieving and responding to commands
//					from the MCU.
//
//					This file contains all of the Cmd* functions except for
//					CmdDiagTraceConfig() which is located in diag.c because of the number
//					local variables used.
//
// Copyright (C) 2002 Texas Instruments Incorporated
// Texas Instruments Proprietary Information
// Use subject to terms and conditions of TI Software License Agreement
//
// Revision History:
// 08/xx/04	HEM		Copied file from CAN project.
// 09/xx/04	HEM		Removed CAN functions.
//==========================================================================================

#include "main.h"
#include <string.h>

//==========================================================================================
// Local function prototypes
//==========================================================================================
u16 CmdReadMemory(void);
u16 CmdWriteMemory(void);
u16 CmdReadStats(void);
u16 CmdConfigFlooder(void);
u16 CmdPLCCommand(void);
u16 CmdLamp(void);
//u16 CmdConfigCorrupter(void);
u16 CmdLocalAddress(void);
u16 CmdPLCEchoSet(void);
u16 CmdPLCEcho(void);
u16 CmdPLCEchoAck(void);

//==========================================================================================
// Local variables
//==========================================================================================

u16	uLampIgniteState = 0;

//==========================================================================================
// External Variables
//==========================================================================================


//==========================================================================================
// Local constants
//==========================================================================================



//==========================================================================================
// Function:		TaskCommand()
//
// Description: 	This task actually executes commands and responds to the MCU.
//              	This task gets posted after an MCU interrupt specifying a command
//					occurs.  This function reads the parms acknowledges the command,
//					then executes the command and responds to the MCU when complete.
//
//					While primarily intended for commands received from the UART,
//					commands may also be received from the PLC.
//
// Revision History:
// 09/17/04	HEM		New command CmdPlcCommand().
// 09/22/04	HEM		New command CmdLamp.
// 11/17/04	HEM		Removed trace trigger code.
//==========================================================================================
void TaskCommand(void)
{
	u16		i;		// Loop index
	
	// Read in the command from the serial buffer if not already done.
	if (uCommandPending == 1)
	{
		// Copy command structure from UART Serial command structure
		for(i=0; i<COMMAND_PARMS; i++)
		{
			upCommand[i] = upSerialCommand[i];
		}
		uCommandPending = 0;
	}

	switch (upCommand[NUMBER])
	{
	case CMD_READ_MEMORY:
		CmdReadMemory();
		break;

	case CMD_WRITE_MEMORY:
		CmdWriteMemory();
		break;

	case CMD_READ_STATS:
		CmdReadStats();
		break;

	case CMD_LAMP_DIRECT:
	case CMD_LAMP:
		CmdLamp();
		break;

	case CMD_PLC_COMMAND:
		CmdPLCCommand();		
		break;

	case CMD_CONFIG_FLOODER:
		CmdConfigFlooder();
		break;

	case CMD_ECHO_SET:
		CmdPLCEchoSet();
		break;
		
	case CMD_ECHO_CMD:
		CmdPLCEcho();
		break;

	case CMD_ECHO_ACK:
		CmdPLCEchoAck();
		break;

	case CMD_LOCAL_ADDRESS:
		CmdLocalAddress();
		break;

	default:	// An unrecognized command was received.
		WriteUARTValue(ERR_UKNOWN_COMMAND);		// Return error code to the uart.
		uCommandActive = 0;						// Nothing to do - clear flag.
		// Rest of cleanup for next command will be handled below as if a normal command
		// had been completed.
		break;	
	}								// End switch (CommandNumber)

	// Check to see if the command has completed.
	// (Individual command functions will clear this flag when they're done.)
	// If the command is done, clean up any necesary trace buffer functions, turn off the
	// Command LED (if present), and do anything else which turns out to be useful.
	if (uCommandActive == 0)
	{
		#if (TRACE_BUF_LEN > 0)	
		// Reset the trace buffer triggered bit, if it was set to stop at command complete.
		// The second condition, prevents the end of "CMD_DIAG_TRACE_CONFIG" from disabling
		// the trace before it ever gets started.
		if ( (uTraceStopCond == TSTOP_CMD_CMPL) && 
		     (upCommand[NUMBER] != CMD_DIAG_TRACE_CONFIG) )
		{
			ClearBits (uTraceStatus, TS_TRIGGERED);
			// If the trace is not supposed to re-start, disable also.
			if (TestBits(uTraceStatus, TS_HALT_TRIG, TS_HALT_TRIG))
			{
				ClearBits(uTraceStatus, TS_ENABLE);
			}
		}
		#endif		
	}
}


//==========================================================================================
// Function:		CmdLocalAddress()
//
// Description: 	DSP Command "Get Local Address"
//
// Revision History:
// 01/25/05 Hagen	made new
//==========================================================================================
u16 CmdLocalAddress(void)
{

	WriteUARTValue( SUCCESS );
	WriteUARTValue( uMyAddress );

	uCommandActive = 0;		// Command is done.  Allow TaskCommand to finish up.
	return (SUCCESS);
}


//==========================================================================================
// Function:		CmdReadMemory()
//
// Description: 	DSP Command "Read Memory"
//
//					Parm #	Description
//						0	Command number = 0001h
//						1	Control flags
//							bit 15	bit 14	bit 13	bit 12	bit 11	bit 10	bit 9	bit 8
//							unused	unused	unused	unused	unused	unused	unused	unused	
//							bit 7	bit 6	bit 5	bit 4	bit 3	bit 2	bit 1	bit 0
//							unused	unused	unused	unused	unused	unused	RM_MODE	unused
//						2	Count
//					 3-31	Address specification list.
//
//					The RM_MODE bits determine whether addresses are specified individually
//					or as a block.
//						RM_MODE_ADDR  = Addresses specified in Parms 3-31
//						RM_MODE_BLOCK = Block read.  Read starting at address in Parm3.
//
//					Data is returned via the WriteUART() and WriteUARTValue() funtions.
//
//					NOTE: As a Cmd* function, this should only be called as the result of a
//							command.  Code which needs to be called from other functions
//							should be split into a new function.
//
// Revision History:
// 06/24/02 EGO		Reflect changes for TEC from Jervis.
// 07/18/02 EGO		Removed support for reading program memory.
//==========================================================================================
u16 CmdReadMemory(void)
{
	u16		uStatus = SUCCESS;			// Return value
	u16		i;							// loop index
	s16		*spTarget;					// pointer to target word


	// For serial commands the return code has to be sent before the data,
	// so the best we can do is send the return code after the command parms
	// have been validated.  If the command fails, the return code will be sent
	// at the end of this condition.

	// In address mode, the addresses to read are specified individually in the passed
	// parameter list.  RM_COUNT determines the number of addresses to read which is
	// limited by the number of parms.
	if (TestBits(upCommand[FLAG], RM_MODE, RM_MODE_ADDR))
	{
		if (upCommand[RM_COUNT] > RM_ADDR_MAX)
		{
			uStatus = ERR_RM_INVALID_COUNT;
		}
		else
		{
			WriteUARTValue(SUCCESS);
			for (i=0; i<upCommand[RM_COUNT]; i++)
			{
				spTarget = (s16 *) upCommand[RM_ADDRESS+i];
				WriteUARTValue((u16) (*spTarget) );
			}
		}
	}

	// In block mode, the addresses to read are sequential starting at the address
	// specified in the parameter list.  RM_COUNT determines the number of addresses
	// to read.
	else // RM_MODE == RM_MODE_BLOCK
	{
		WriteUARTValue(SUCCESS);
		WriteUART(upCommand[RM_COUNT], (u16*)upCommand[RM_ADDRESS]);
	}	// end of block mode.
		
	if (uStatus != SUCCESS)
	{
		WriteUARTValue(uStatus);
	}

	// Command is done.  Allow TaskCommand to finish up.
	uCommandActive = 0;

	return (uStatus);
}


/*==========================================================================================
Function:		CmdReadStats()
Description: 	DSP Command "Read Status"
				Parm #	Description
					0	Command number = 0003h
					1	reset flag
						1 ==  reset status counters
						else  read status counters

				returned values
				 	byte description
				 	0	 return code
				 	1	 count
				 	2-N	 statistics data	

				Data is returned via the WriteUART() and WriteUARTValue() funtions.
				NOTE: As a Cmd* function, this should only be called as the result of a
						command.  Code which needs to be called from other functions
						should be split into a new function.

Revision History:
17Feb05	 Hagen		Made from CmdReadMemory()
==========================================================================================*/
u16 CmdReadStats(void)
{

	if( upCommand[1] == 1 ) 
	{
		memset( (u16*)ulPlcStats, 0, PLC_STATS_LEN*sizeof(u16) );
		memset( (u16*)ulBerStats, 0, BER_STATS_LEN*sizeof(u16) );
		WriteUARTValue( SUCCESS );
	}
	else
	{
		// For serial commands the return code has to be sent before the data.
		WriteUARTValue( SUCCESS );
		WriteUARTValue( PLC_STATS_LEN + BER_STATS_LEN );
		WriteUART( PLC_STATS_LEN, (u16*)ulPlcStats );
		WriteUART( BER_STATS_LEN, (u16*)ulBerStats );
	}
	
	// Command is done.  Allow TaskCommand to finish up.
	uCommandActive = 0;

	return ( SUCCESS );
}


//==========================================================================================
// Function:		CmdWriteMemory()
//
// Description: 	DSP Command "Write Memory"
//					The Write Memory command is used to modify the internal DSP memory.
//
//					Parm #	Description
//						0	Command number = 0002h
//						1	Control flags
//							bit 15	bit 14	bit 13	bit 12	bit 11	bit 10	bit 9	bit 8
//							unused	unused	unused	unused	unused	unused	unused	unused	
//							bit 7	bit 6	bit 5	bit 4	bit 3	bit 2	bit 1	bit 0
//							unused	unused	unused	unused	unused	unused	WM_MODE	unused
//						2	Count
//					 3-31	These parms are split into two word pairs:
//						     Low word:  Address
//						     High word:  Data
//
//					The WM_MODE bits determine whether addresses are specified individually
//					or if the memory is going to be block filled.
//						WM_MODE_ADDR  = Addresses specified in Parms 3-31
//							WM_COUNT specified number of address/data pairs to follow.
//						WM_MODE_BLOCK = Block fill.
//							WM_ADDRESS specifies starting address.
//							WM_COUNT specifies number of words to write.
//							WM_DATA specifies fill value.
//
//
//					NOTE: As a Cmd* function, this should only be called as the result of a
//							command.  Code which needs to be called from other functions
//							should be split into a new function.
//
// Revision History:
// 06/25/02 EGO		Reflect changes for TEC from Jervis.
// 07/18/02 EGO		Removed support for reading program memory.
//==========================================================================================
u16 CmdWriteMemory(void)
{
	u16		uStatus = SUCCESS;			// Return value.
	u16		i;							// loop index
	s16		*spTarget;					// pointer to target word
	

	// In address mode, the addresses and data to be written are specified individually
	// in the passed parameter list.  WM_COUNT determines the number of pairs to write,
	// limited by the number of parms.
	if (TestBits(upCommand[FLAG], WM_MODE, WM_MODE_ADDR))
	{
		if ((upCommand[WM_COUNT] > WM_ADDR_MAX))
		{
			uStatus = ERR_WM_INVALID_COUNT;
		}
		else
		{
			for (i=0; i<upCommand[WM_COUNT]; i++)
			{
				spTarget = (s16 *) upCommand[WM_ADDRESS+(i*2)];
				*spTarget = upCommand[WM_DATA+(i*2)];
			}
		}
	}

	// In block mode, the addresses to be written are sequential starting at the address
	// specified in the parameter list.  WM_COUNT determines the number of addresses
	// to write.  There is no limit to Count.
	else // WM_MODE == WM_MODE_BLOCK
	{                                                          
		for (i=0; i<upCommand[WM_COUNT]; i++)
		{
			spTarget = (s16 *) (upCommand[WM_ADDRESS] + i);
			*spTarget = upCommand[WM_DATA];
		}
	}

	WriteUARTValue(uStatus);

	// Command is done.  Allow TaskCommand to finish up.
	uCommandActive = 0;

	return (uStatus);
}


//==========================================================================================
// Function:		CmdPLCCommand()
//
// Description: 	This function allows a host connected to this board via RS-232 to 
//					issue commands over the power line.
///					Parm #	Description
//						0	Command number = 000Ch
//					 1-31	PLC Message content
//
//					NOTE: As a Cmd* function, this should only be called as the result of a
//							command.  Code which needs to be called from other functions
//							should be split into a new function.
//
// Revision History:
// 09/17/04 HEM		New function, copied from CmdCANCommand.
//==========================================================================================
u16 CmdPLCCommand(void)
{
	u16	uStatus = SUCCESS;		// Return value.
	u16	i;						// Generic loop index

	// Copy from UART command buffer into outgoing PLC data buffer
	for (i=1; i<COMMAND_PARMS; i++)
	{
		txUserDataArray[i-1] = upCommand[i];	//one byte per word
	}

	uTxMsgPending = True;		// Set flag to tell main loop to start sending message when traffic permits

	WriteUARTValue(uStatus);	// Respond status to UART

	// Command is done.  Allow TaskCommand to finish up.
	uCommandActive = 0;

	return (uStatus);
}


//==========================================================================================
// Function:		CmdConfigFlooder()
//
// Description: 	This function sets the flood rate at which Fake PLC messages are generated.
//
// Revision History:
// 10/05/04 HEM		New function.
//==========================================================================================
u16 CmdConfigFlooder(void)
{
	u16	uStatus = SUCCESS;		// Return value.
	if (upCommand[1] == 0)
		{
			uFloodInterval = 0;		// Turn off flood	
		}
		else
		{
			uFloodInterval = TINTS_PER_SEC/Saturate(upCommand[1], 1, 25);	// Limit flood rate
		}
		
	WriteUARTValue(uStatus);

	// Command is done.  Allow TaskCommand to finish up.
	uCommandActive = 0;

	return (uStatus);
}


//==========================================================================================
// Function:		CmdPLCEchoSet()
//
// Description: 	Define modem properties to setup sending packets to measure BER
//					Parm #	Description
//						0	Command number = 0020h
//						1	Control flags
//							bit 15-8	unused
//							bit  7-2 	unused
//							bit  1		BER_READ	Return berstats[] to PC
//							bit  0		BER_RESET	Clear berstats[]
//					    2	slave address
//					    3	Flood rate (1 to 15 Hz)
//					 4-31 	don't care
//
// Globals:
//		u16	upCommand[COMMAND_PARMS];  // command message
//		u16	uTxMsgPending;
//		u16	txUserDataArray[MAX_TX_MSG_LEN];// byte-wide buffer for user data
//		u16	uCommandActive;
//		u16	uMyAddress;
//		u16	uDestAddress;
//
// Revision History:
// 01/17/05 Hagen	New function
//==========================================================================================
u16 CmdPLCEchoSet(void)
{
	u16		uStatus = SUCCESS;			// Return value
	u16		n;							// loop index


	uDestAddress = upCommand[2];		// set address to send to BER packets to
	//uDestAddress = (upCommand[2]<<8) | upCommand[3])

	if( upCommand[3] == 0 )
		uFloodInterval = 0;		// Turn off flood	
	else
		uFloodInterval = TINTS_PER_SEC/Saturate(upCommand[3], 1, 25);	// Limit flood rate

	//---- read counters -----------------------------------
	if( TestBits(upCommand[FLAG], BER_READ, BER_READ) )
	{
		WriteUARTValue(uStatus);
		WriteUART(BER_STATS_LEN, (u16*)ulBerStats);
	}	// end of block mode.
	else
	{
		WriteUARTValue(uStatus);
	}

	//---- reset counters -----------------------------------
	if( TestBits(upCommand[FLAG], BER_RESET, BER_RESET) )
	{
		for( n = 0; n < BER_STATS_LEN; n++ )
		{
			ulBerStats[n] = 0;
		}
	}
		
	// Command is done.  Allow TaskCommand to finish up.
	uCommandActive = 0;

	return (uStatus);
}



//==========================================================================================
// Function:		CmdPLCEcho()
//
// Description: 	When encountering this command, send a response on the powerline 
//					format of packet that came in:
//					   byte	description
//						 	my address high byte	(stripped off by ProcessRxPlcMsg)
//						 	my address low byte		(stripped off by ProcessRxPlcMsg)
//						0	echo command (0020h)	(Checked in TaskCommand)
//						1	master address high byte
//						2	master address low byte
//						3	ack command (0021h)
//					30-31	CRC	
//					format of packet going out:
//					   byte	description
//						0	master address high byte 	(byte3)
//						1	master address low byte  	(byte4)
//						2	ack command (0021h)			(byte5)
//						3	my address high byte
//						4	my address low byte
//						5	return code
//					30-31	CRC	
//
// Globals:
//		u16	upCommand[COMMAND_PARMS];  // command message
//		u16	txUserDataArray[MAX_TX_MSG_LEN];// byte-wide buffer for user data
//		u16	uMyAddress;
//		u16	uTxMsgPending;
//		u16	uCommandActive;
//
// Revision History:
// 01/17/05 Hagen	New function
//==========================================================================================
u16 CmdPLCEcho(void)
{
	u16	uStatus = SUCCESS;		// Return value.
	u16 *txUserData;

	//---- Copy from command buffer into outgoing PLC data buffer -----
	txUserData = txUserDataArray;
	*txUserData++ = upCommand[1];
	*txUserData++ = upCommand[2];
	*txUserData++ = upCommand[3];
	*txUserData++ = uMyAddress>>8;
	*txUserData++ = uMyAddress& 0x00FF;
	*txUserData++ = uStatus;				// return code
	
	uTxMsgPending = True;	// Set flag to tell main loop to start sending message when traffic permits
	uCommandActive = 0;		// Command is done.  Allow TaskCommand to finish up.
	ulBerStats[BER_ECHO_COUNT]++;				// Add more here!!!

	return (uStatus);
}


//==========================================================================================
// Function:		CmdPLCEchoAck()
//
// Description: 	Should get this command back after sending a CmdPLCEcho.  
//					increment counter when we get it.
//					Format of packet that came in:
//					   byte	description
//						 	my address high byte	(stripped off by ProcessRxPlcMsg)
//						 	my address low byte		(stripped off by ProcessRxPlcMsg)
//						0	ack command (0021h)		(Checked in TaskCommand)
//						1	from address high byte
//						2	from address low byte
//						3	return code
//					30-31	CRC	
//
// Globals:
//		u16	upCommand[COMMAND_PARMS];  // command message
//		u16	txUserDataArray[MAX_TX_MSG_LEN];// byte-wide buffer for user data
//		u16	uMyAddress;
//		u16	uTxMsgPending;
//		u16	uCommandActive;
//
// Revision History:
// 01/17/05 Hagen	New function
//==========================================================================================
u16 CmdPLCEchoAck(void)
{
	u16		uStatus = SUCCESS;		// Return value.
	u16		uFromAddress;
	
	
	uFromAddress =  (upCommand[1]<<8) + upCommand[2];
	if( uFromAddress == uDestAddress )
	{
		uCmd_EchoAck = True;
	
		if( upCommand[3] == 0 )
			ulBerStats[BER_ACK_COUNT]++;				// Add more here!!!
		else
			ulBerStats[BER_NZERO_COUNT]++;				// Add more here!!!
	}
	
	
	uCommandActive = 0;		// Command is done.  Allow TaskCommand to finish up.

	return (uStatus);
}


//==========================================================================================
// Function:		InitLampVars()
//
// Description: 	This function initializes the lamp command variables to their reset values.
//
// Revision History:
// 09/23/04 HEM		New function.
//==========================================================================================
#define	LAMP_PHYSICAL_MIN_LEVEL	1		// This number is product-specific
#define	LAMP_PHYSICAL_MAX_LEVEL	254		// This number is always 254
void InitLampVars(void)
{
	u16	i;
	
	ubLampIntensityTarget = LAMP_PHYSICAL_MAX_LEVEL;
	ubLampIntensity = LAMP_PHYSICAL_MAX_LEVEL;
	ubLampDTR = ubLampIntensity;				
	ubLampMaxLevel = LAMP_PHYSICAL_MAX_LEVEL;
	ubLampMinLevel = LAMP_PHYSICAL_MIN_LEVEL;	
	ubLampSystemFailureLevel = LAMP_PHYSICAL_MAX_LEVEL;
	ubLampPowerOnLevel = LAMP_PHYSICAL_MAX_LEVEL;
	uLampGroupFlags = 0;
	ubLampFadeTime = 0;
	ubLampFadeRate = 7;
	uFadeInterval = 0;
	for (i= 0; i<16; i++)
	{
		ubLampScene[i] = 255;	// Set all scenes to ignore requests until programmed
	}
}


//==========================================================================================
// Function:		CmdLamp()
//
// Description: 	This function adjusts the intensity of the lamp.
//					Parm #	Description
//						0	Command number = 000Ah (Direct Lamp Power Control)
//						1	Lamp Intensity
//					--- OR ---
//						0	Command number = 000Bh	(Lamp Commands, similar to DALI spec)
//						1	LAMP Command
//
//					NOTE: As a Cmd* function, this should only be called as the result of a
//							command.  Code which needs to be called from other functions
//							should be split into a new function.
//
// Revision History:
// 09/23/04 HEM		New function.
//==========================================================================================
enum {	LAMP_OFF,				// 0 
		LAMP_UP,				// 1
		LAMP_DOWN,				// 2
		LAMP_STEP_UP, 			// 3
		LAMP_STEP_DOWN,			// 4
		LAMP_RECALL_MAX_LEVEL,	// 5
		LAMP_RECALL_MIN_LEVEL,	// 6
		LAMP_STEP_DOWN_AND_OFF,	// 7
		LAMP_ON_AND_STEP_UP,	// 8
		LAMP_RSVD9, LAMP_RSVD10, LAMP_RSVD11, LAMP_RSVD12, LAMP_RSVD13, LAMP_RSVD14, LAMP_RSVD15, 	// 9-15
		LAMP_GO_TO_SCENE0,  LAMP_GO_TO_SCENE1,  LAMP_GO_TO_SCENE2,  LAMP_GO_TO_SCENE3, 			// 16-19
		LAMP_GO_TO_SCENE4,  LAMP_GO_TO_SCENE5,  LAMP_GO_TO_SCENE6,  LAMP_GO_TO_SCENE7, 			// 20-23
		LAMP_GO_TO_SCENE8,  LAMP_GO_TO_SCENE9,  LAMP_GO_TO_SCENE10, LAMP_GO_TO_SCENE11, 		// 24-27
		LAMP_GO_TO_SCENE12, LAMP_GO_TO_SCENE13, LAMP_GO_TO_SCENE14, LAMP_GO_TO_SCENE15, 		// 28-31
		LAMP_RESET,									// 32
		LAMP_STORE_ACTUAL_LEVEL_IN_THE_DTR,			// 33
		LAMP_RSVD34, LAMP_RSVD35, LAMP_RSVD36, LAMP_RSVD37, // 34-37 Reserved
		LAMP_RSVD38, LAMP_RSVD39, LAMP_RSVD40, LAMP_RSVD41, // 38-41 Reserved
		LAMP_STORE_THE_DTR_AS_MAX_LEVEL,			// 42  
		LAMP_STORE_THE_DTR_AS_MIN_LEVEL,			// 43
		LAMP_STORE_THE_DTR_AS_SYSTEM_FAILURE_LEVEL,	// 44
		LAMP_STORE_THE_DTR_AS_POWER_ON_LEVEL,		// 45
		LAMP_STORE_THE_DTR_AS_FADE_TIME,			// 46
		LAMP_STORE_THE_DTR_AS_FADE_RATE,			// 47		
		LAMP_RSVD48, LAMP_RSVD49, LAMP_RSVD50, LAMP_RSVD51, // 48-51 Reserved
		LAMP_RSVD52, LAMP_RSVD53, LAMP_RSVD54, LAMP_RSVD55, // 52-55 Reserved
		LAMP_RSVD56, LAMP_RSVD57, LAMP_RSVD58, LAMP_RSVD59, // 56-59 Reserved
		LAMP_RSVD60, LAMP_RSVD61, LAMP_RSVD62, LAMP_RSVD63,  // 60-63 Reserved		
		LAMP_STORE_THE_DTR_AS_SCENE0,  LAMP_STORE_THE_DTR_AS_SCENE1,	// 64-65							
		LAMP_STORE_THE_DTR_AS_SCENE2,  LAMP_STORE_THE_DTR_AS_SCENE3,	// 66-67							
		LAMP_STORE_THE_DTR_AS_SCENE4,  LAMP_STORE_THE_DTR_AS_SCENE5,	// 68-69							
		LAMP_STORE_THE_DTR_AS_SCENE6,  LAMP_STORE_THE_DTR_AS_SCENE7,	// 70-71							
		LAMP_STORE_THE_DTR_AS_SCENE8,  LAMP_STORE_THE_DTR_AS_SCENE9,	// 72-73							
		LAMP_STORE_THE_DTR_AS_SCENE10, LAMP_STORE_THE_DTR_AS_SCENE11,	// 74-75											
		LAMP_STORE_THE_DTR_AS_SCENE12, LAMP_STORE_THE_DTR_AS_SCENE13,	// 76-77							
		LAMP_STORE_THE_DTR_AS_SCENE14, LAMP_STORE_THE_DTR_AS_SCENE15,	// 78-79							
		LAMP_REMOVE_FROM_SCENE0,  LAMP_REMOVE_FROM_SCENE1,				// 80-81						
		LAMP_REMOVE_FROM_SCENE2,  LAMP_REMOVE_FROM_SCENE3,				// 82-83						
		LAMP_REMOVE_FROM_SCENE4,  LAMP_REMOVE_FROM_SCENE5,				// 84-85						
		LAMP_REMOVE_FROM_SCENE6,  LAMP_REMOVE_FROM_SCENE7,				// 86-87						
		LAMP_REMOVE_FROM_SCENE8,  LAMP_REMOVE_FROM_SCENE9,				// 88-89						
		LAMP_REMOVE_FROM_SCENE10, LAMP_REMOVE_FROM_SCENE11,				// 90-91									
		LAMP_REMOVE_FROM_SCENE12, LAMP_REMOVE_FROM_SCENE13,				// 92-93						
		LAMP_REMOVE_FROM_SCENE14, LAMP_REMOVE_FROM_SCENE15,				// 94-95						
		LAMP_ADD_TO_GROUP0,  LAMP_ADD_TO_GROUP1,						// 96-97
		LAMP_ADD_TO_GROUP2,  LAMP_ADD_TO_GROUP3,						// 98-99
		LAMP_ADD_TO_GROUP4,  LAMP_ADD_TO_GROUP5,						// 100-101
		LAMP_ADD_TO_GROUP6,  LAMP_ADD_TO_GROUP7,						// 102-103
		LAMP_ADD_TO_GROUP8,  LAMP_ADD_TO_GROUP9,						// 104-105
		LAMP_ADD_TO_GROUP10, LAMP_ADD_TO_GROUP11,						// 106-107
		LAMP_ADD_TO_GROUP12, LAMP_ADD_TO_GROUP13,						// 108-109
		LAMP_ADD_TO_GROUP14, LAMP_ADD_TO_GROUP15,						// 110-111
		LAMP_REMOVE_FROM_GROUP0,LAMP_REMOVE_FROM_GROUP1,				// 112-113
		LAMP_REMOVE_FROM_GROUP2,LAMP_REMOVE_FROM_GROUP3,				// 114-115
		LAMP_REMOVE_FROM_GROUP4,LAMP_REMOVE_FROM_GROUP5,				// 116-117
		LAMP_REMOVE_FROM_GROUP6,LAMP_REMOVE_FROM_GROUP7,				// 118-119
		LAMP_REMOVE_FROM_GROUP8,LAMP_REMOVE_FROM_GROUP9,				// 120-121
		LAMP_REMOVE_FROM_GROUP10,LAMP_REMOVE_FROM_GROUP11,				// 122-123
		LAMP_REMOVE_FROM_GROUP12,LAMP_REMOVE_FROM_GROUP13,				// 124-125
		LAMP_REMOVE_FROM_GROUP14,LAMP_REMOVE_FROM_GROUP15				// 126-127				
		};

// Number of TINT interrupts required to perform fade to new intensity level
// Formula = TINTS_PER_SEC / 2 * sqrt(2^X), with X=1..15
// This table was built with TINTS_PER_SEC == 2000
u32	ulFadeIntervalTable[16] = 	{ 1000,
								  1414,
								  2000,
								  2828,
								  4000,
								  5657,
								  8000,
								 11314,
								 16000,
								 22627,
								 32000,
								 45255,
								 64000,
								 90510,
								128000,
								181019};

// Fade Step Table.  Number of fader steps that take place in 200 ms for CMD1(UP) and CMD2(DOWN)
u16 ubFadeStepTable[16]=	{	101,	
								 72,	
								 51,	
								 36,	
								 25,	
								 18,	
								 13,	
								  9,	
								  6,  	
								  4,	
								  3,	
								  2,	
								  2,	
								  1,	
								  1,	
								  1};

		
u16	CmdLamp(void)
{
	u16	uStatus = SUCCESS;		// Return value.
	u16	uLampCmd;
	
		if (upCommand[0] == CMD_LAMP_DIRECT)
		{
			if (upCommand[1] == 0)				// Dim down to MinLevel, then shut off
			{
				ubLampIntensityTarget = 0;		
				if(ubLampIntensity != ubLampMinLevel)
				{
					uFadeInterval = ulFadeIntervalTable[ubLampFadeTime] / abs(ubLampIntensity-ubLampMinLevel);
				}
				else
				{
					uFadeInterval = 0;
				}
			}
			else if (upCommand[1] == 255)		// Cease any dimming in progress and hold at present level
			{	
				ubLampIntensityTarget = ubLampIntensity;
			}
			else								// Dim to new level
			{
				ubLampIntensityTarget = Saturate(upCommand[1], ubLampMinLevel, ubLampMaxLevel);
				if(ubLampIntensity != ubLampIntensityTarget)
				{
					uFadeInterval = ulFadeIntervalTable[ubLampFadeTime] / abs(ubLampIntensityTarget - ubLampIntensity);
				}
				else
				{
					uFadeInterval = 0;
				}
			}
		}
		else
		{	
			uLampCmd = upCommand[1];
			switch (uLampCmd)
			{
			case LAMP_OFF:
				ubLampIntensityTarget = 0;
					uFadeInterval = 0;
				break;
				
			case LAMP_UP:					// 1 Fade Up if already lit
				if (ubLampIntensity > 0)
				{
					ubLampIntensityTarget = Saturate(ubLampIntensity+ubFadeStepTable[ubLampFadeRate], ubLampMinLevel, ubLampMaxLevel);
//					uFadeInterval = uFadeIntervalTable[ubLampFadeRate];
				    uFadeInterval = Max(1, (253UL * ulFadeIntervalTable[2]) / (ulFadeIntervalTable[ubLampFadeRate] * 5));
				}
				break;

			case LAMP_DOWN:					// 2 Fade down if already lit.  Do not turn off.
				if (ubLampIntensity > 0)
				{
					ubLampIntensityTarget = Saturate((q16)ubLampIntensity-(q16)ubFadeStepTable[ubLampFadeRate], (q16)ubLampMinLevel, (q16)ubLampMaxLevel);
//					uFadeInterval = uFadeIntervalTable[ubLampFadeRate];
				    uFadeInterval = Max(1, (253UL * ulFadeIntervalTable[2]) / (ulFadeIntervalTable[ubLampFadeRate] * 5));
				}				
				break;
				
			case LAMP_STEP_UP: 				// 3 Step up one step if already lit
				if (ubLampIntensity > 0)
				{
					ubLampIntensityTarget = Saturate(ubLampIntensity+1, ubLampMinLevel, ubLampMaxLevel);
					uFadeInterval = 0; 		// Step as soon as possible
				}
				break;
				
			case LAMP_STEP_DOWN:			// 4 Step down one step if already lit
				if (ubLampIntensity > 0)
				{
					ubLampIntensityTarget = Saturate(ubLampIntensity-1, ubLampMinLevel, ubLampMaxLevel);
					uFadeInterval = 0; 		// Step as soon as possible
				}				
				break;
				
			case LAMP_RECALL_MAX_LEVEL:		// 5 Set intensity to MaxLevel, without fading
				if (ubLampIntensity == 0)	// If lamp was off, ignite it
				{
					uLampIgniteState = 1;		
				}
				ubLampIntensityTarget = ubLampMaxLevel;
				ubLampIntensity = ubLampIntensityTarget;
				uFadeInterval = 0; 		// Step as soon as possible
				break;
				
			case LAMP_RECALL_MIN_LEVEL:		// 6 Set intensity to MinLevel, without fading
				if (ubLampIntensity == 0)	// If lamp was off, ignite it
				{
					uLampIgniteState = 1;		
				}
				ubLampIntensityTarget = ubLampMinLevel;
				ubLampIntensity = ubLampIntensityTarget;
				uFadeInterval = 0; 		// Step as soon as possible
				break;
				
			case LAMP_STEP_DOWN_AND_OFF:	// 7 Step down one step.  Turn off if already at MinLevel
				if (ubLampIntensity <= ubLampMinLevel)
				{
					ubLampIntensityTarget = 0;				 
				}
				else
				{
					ubLampIntensityTarget = ubLampIntensity-1;				 
				}
				uFadeInterval = 0; 		// Step as soon as possible
				break;
				
			case LAMP_ON_AND_STEP_UP:		// 8
				if (ubLampIntensity == 0)	// If lamp was off, ignite it
				{
					uLampIgniteState = 1;		
				}
				if ((ubLampIntensity < ubLampMinLevel) || (ubLampIntensity == 0))
				{
					ubLampIntensityTarget = ubLampMinLevel;
				}
				else
				{
					ubLampIntensityTarget = Saturate(ubLampIntensity+1, ubLampMinLevel, ubLampMaxLevel);
				}
				uFadeInterval = 0; 		// Step as soon as possible
				break;
	
			case LAMP_GO_TO_SCENE0:  
			case LAMP_GO_TO_SCENE1:  
			case LAMP_GO_TO_SCENE2:  
			case LAMP_GO_TO_SCENE3: 		
			case LAMP_GO_TO_SCENE4:  
			case LAMP_GO_TO_SCENE5:  
			case LAMP_GO_TO_SCENE6:  
			case LAMP_GO_TO_SCENE7: 		
			case LAMP_GO_TO_SCENE8:  
			case LAMP_GO_TO_SCENE9:  
			case LAMP_GO_TO_SCENE10: 
			case LAMP_GO_TO_SCENE11: 		
			case LAMP_GO_TO_SCENE12: 
			case LAMP_GO_TO_SCENE13: 
			case LAMP_GO_TO_SCENE14: 
			case LAMP_GO_TO_SCENE15: 
				{
					//u16	uScene = uLampCmd & 0x0F;
					//ubLampIntensityTarget = Saturate(ubLampScene[uScene], ubLampMinLevel, ubLampMaxLevel);		
					u8	ubSceneLevel = ubLampScene[uLampCmd & 0x0F];
					if (ubSceneLevel == 255)	// Ignore the request if the level setting for this scene is 255
					{
						NOP;		// Do nothing
					}
					else if (ubSceneLevel == 0)	// Fade down to minimum level, then turn off
					{
						ubLampIntensityTarget = 0;		
						if (ubLampIntensity != ubLampMinLevel)
						{
							uFadeInterval = ulFadeIntervalTable[ubLampFadeTime] / abs(ubLampIntensity-ubLampMinLevel);
						}
						else
						{
							uFadeInterval = 0;
						}	
					}
					else						// Fade to new scene level
					{						
						ubLampIntensityTarget = Saturate(ubSceneLevel, ubLampMinLevel, ubLampMaxLevel);							
		
						if (ubLampIntensity == 0)	// If lamp was off, ignite the lamp
						{
							uLampIgniteState = 1;		
						}
	
						if (ubLampIntensity != ubLampIntensityTarget)
						{
							uFadeInterval = ulFadeIntervalTable[ubLampFadeTime] / abs(ubLampIntensityTarget - ubLampIntensity);
						}
						else
						{
							uFadeInterval = 0;
						}
					}
				}
				break;

			case LAMP_RESET:								// 32
				InitLampVars();
				break;
				
			case LAMP_STORE_ACTUAL_LEVEL_IN_THE_DTR:		// 33
				ubLampDTR = ubLampIntensity;				// Lamp Controller Data Transfer Register
				break;
	
			case LAMP_STORE_THE_DTR_AS_MAX_LEVEL:			// 42 
				ubLampMaxLevel = ubLampDTR;
				break;
				
			case LAMP_STORE_THE_DTR_AS_MIN_LEVEL:			// 43
				ubLampMinLevel = ubLampDTR;
				break;
				
			case LAMP_STORE_THE_DTR_AS_SYSTEM_FAILURE_LEVEL:// 44
				ubLampSystemFailureLevel = ubLampDTR;
				break;
				
			case LAMP_STORE_THE_DTR_AS_POWER_ON_LEVEL:		// 45
				ubLampPowerOnLevel = ubLampDTR;
				break;
				
			case LAMP_STORE_THE_DTR_AS_FADE_TIME:			// 46
				ubLampFadeTime = ubLampDTR;
				break;
				
			case LAMP_STORE_THE_DTR_AS_FADE_RATE:			// 47		
				ubLampFadeRate = ubLampDTR;
				break;
			
			case LAMP_STORE_THE_DTR_AS_SCENE0:				// 64
			case LAMP_STORE_THE_DTR_AS_SCENE1:				// 65							
			case LAMP_STORE_THE_DTR_AS_SCENE2:				// 66		
			case LAMP_STORE_THE_DTR_AS_SCENE3:				// 67							
			case LAMP_STORE_THE_DTR_AS_SCENE4:				// 68		
			case LAMP_STORE_THE_DTR_AS_SCENE5:				// 69							
			case LAMP_STORE_THE_DTR_AS_SCENE6:				// 70		
			case LAMP_STORE_THE_DTR_AS_SCENE7:				// 71							
			case LAMP_STORE_THE_DTR_AS_SCENE8:				// 72		
			case LAMP_STORE_THE_DTR_AS_SCENE9:				// 73							
			case LAMP_STORE_THE_DTR_AS_SCENE10:				// 74
			case LAMP_STORE_THE_DTR_AS_SCENE11:				// 75											
			case LAMP_STORE_THE_DTR_AS_SCENE12:				// 76		
			case LAMP_STORE_THE_DTR_AS_SCENE13:				// 77							
			case LAMP_STORE_THE_DTR_AS_SCENE14:				// 78		
			case LAMP_STORE_THE_DTR_AS_SCENE15:				// 79							
				ubLampScene[uLampCmd & 0x0F] = ubLampDTR;	// Store DTR level as intensity setting for this scene
				break;							

			case LAMP_REMOVE_FROM_SCENE0:					// 80
			case LAMP_REMOVE_FROM_SCENE1:					// 81						
			case LAMP_REMOVE_FROM_SCENE2:					// 82	
			case LAMP_REMOVE_FROM_SCENE3:					// 83						
			case LAMP_REMOVE_FROM_SCENE4:					// 84	
			case LAMP_REMOVE_FROM_SCENE5:					// 85						
			case LAMP_REMOVE_FROM_SCENE6:					// 86	
			case LAMP_REMOVE_FROM_SCENE7:					// 87						
			case LAMP_REMOVE_FROM_SCENE8:					// 88	
			case LAMP_REMOVE_FROM_SCENE9:					// 89						
			case LAMP_REMOVE_FROM_SCENE10:					// 90
			case LAMP_REMOVE_FROM_SCENE11:					// 91									
			case LAMP_REMOVE_FROM_SCENE12:					// 92	
			case LAMP_REMOVE_FROM_SCENE13:					// 93						
			case LAMP_REMOVE_FROM_SCENE14:					// 94	
			case LAMP_REMOVE_FROM_SCENE15:					// 95						
				ubLampScene[uLampCmd & 0x0F] = 255;			// Ignore intensity setting for this scene
				break;							

						
			case LAMP_ADD_TO_GROUP0:						// 96
			case LAMP_ADD_TO_GROUP1:						// 97
			case LAMP_ADD_TO_GROUP2:						// 98
			case LAMP_ADD_TO_GROUP3:						// 99
			case LAMP_ADD_TO_GROUP4:						// 100
			case LAMP_ADD_TO_GROUP5:						// 101
			case LAMP_ADD_TO_GROUP6:						// 102	
			case LAMP_ADD_TO_GROUP7:						// 103
			case LAMP_ADD_TO_GROUP8:						// 104
			case LAMP_ADD_TO_GROUP9:						// 105
			case LAMP_ADD_TO_GROUP10:						// 106
			case LAMP_ADD_TO_GROUP11:						// 107
			case LAMP_ADD_TO_GROUP12:						// 108	
			case LAMP_ADD_TO_GROUP13:						// 109
			case LAMP_ADD_TO_GROUP14:						// 110
			case LAMP_ADD_TO_GROUP15:						// 111
				uLampGroupFlags |= (1<<(uLampCmd & 0x0F));
				break;
				
			case LAMP_REMOVE_FROM_GROUP0:					// 112
			case LAMP_REMOVE_FROM_GROUP1:					// 113
			case LAMP_REMOVE_FROM_GROUP2:					// 114
			case LAMP_REMOVE_FROM_GROUP3:					// 115
			case LAMP_REMOVE_FROM_GROUP4:					// 116
			case LAMP_REMOVE_FROM_GROUP5:					// 117
			case LAMP_REMOVE_FROM_GROUP6:					// 118
			case LAMP_REMOVE_FROM_GROUP7:					// 119
			case LAMP_REMOVE_FROM_GROUP8:					// 120
			case LAMP_REMOVE_FROM_GROUP9:					// 121
			case LAMP_REMOVE_FROM_GROUP10:					// 122
			case LAMP_REMOVE_FROM_GROUP11:					// 123
			case LAMP_REMOVE_FROM_GROUP12:					// 124
			case LAMP_REMOVE_FROM_GROUP13:					// 125
			case LAMP_REMOVE_FROM_GROUP14:					// 126
			case LAMP_REMOVE_FROM_GROUP15:					// 127
				uLampGroupFlags &= !(1<<(uLampCmd & 0x0F));
				break;

			
			default:
				break;
		}
	}


	WriteUARTValue(uStatus);	// Respond status to UART
	
	uCommandActive = 0;		// Command is done.  Allow TaskCommand to finish up. 
		
	return (uStatus);		
}

	

//==========================================================================================
// Function:		ControlLamp()
//
// Description: 	This function adjusts the lamp intensity setpoint.
//
// Revision History:
// 09/23/04 HEM		New function.
//==========================================================================================
void ControlLamp(void)
{
	q16	qSlew;
	static	u16 uLampIgniteCntr = 0;	//!!!FAKE
	#define	LAMP_IGNITE_TIMEOUT	200		//!!!FAKE
	
	if (uLampIgniteState != 0)	// Non-zero means ignition is in progress
	{
		SetLampIntensity ((uLampIgniteCntr<<4) & 0xFF);	//!!!FAKE Make the LED look like it is flickering during ignition
		// *** ADD CODE HERE THAT IGNITES THE LAMP AND WAITS FOR THE LAMP TO IGNITE
		// FOR DEMO, WE WILL JUST DO A SHORT DELAY
		if (uLampIgniteCntr++ > LAMP_IGNITE_TIMEOUT)	//!!!FAKE
		{		
			// Lamp is ignited. 
			uLampIgniteState = 0;	
			uLampIgniteCntr = 0;
		}	
	}
	else		
	{
		if (ubLampIntensity != ubLampIntensityTarget)
		{
			// Move the intensity setpoint toward the target value at a rate determined by the Fade Rate and/or Fade Time
			qSlew = (q16)ubLampIntensityTarget - ubLampIntensity;
			qSlew = Saturate(qSlew, -1, 1);
			ubLampIntensity += qSlew;
			ubLampIntensity = Min(ubLampIntensity, ubLampMaxLevel);
			if (ubLampIntensity < ubLampMinLevel)
			{
				if (qSlew < 0)
				{
					ubLampIntensity = 0;
				}
				else
				{
					ubLampIntensity = ubLampMinLevel;
				}
			}
		}
		SetLampIntensity (ubLampIntensity);				
	}
	

}

 
 

//==========================================================================================
// Function:		SetLampIntensity()
//
// Description: 	Set the lamp PWM DAC based on a lamp intensity 
//
// Revision History:
// 09/21/04 HEM		New function.
//==========================================================================================
#if DAC_SIZE == 10
// Lamp Intensity Table from 1 to 255.  
// Log spacing at high intensity, linear spacing at low intensity.
	// Input  1 -->   0.098%
	//        2 -->   0.195%  
	// 		  3 -->   0.293%  
	//		. . .
	//		252	-->	 95.605%
	//		253 -->	 97.070%
	//		254 -->  98.535%
	// 		255 --> 100.000%

	u16	uIntensityTable[256] = 
	{	  0,	  1,	  2,	  3,	  4,	  5,	  6,	  7,	
		  8,	  9,	 10,	 11,	 12,	 13,	 14,	 15,
		 16,	 17,	 18,	 19,	 20,	 21,	 22,	 23,	
		 24,	 25,	 26,	 27,	 28,	 29,	 30,	 31,
		 32,	 33,	 34,	 35,	 36,	 37,	 38,	 39,	
		 40,	 41,	 42,	 43,	 44,	 45,	 46,	 47,
		 48,	 49,	 50,	 51,	 52,	 53,	 54,	 55,	
		 56,	 57,	 58,	 59,	 60,	 61,	 62,	 63,
		 64,	 65,	 66,	 67,	 68,	 69,	 70,	 71,	
		 72,	 73,	 74,	 75,	 76,	 77,	 78,	 79,
		 80,	 81,	 82,	 83,	 84,	 85,	 86,	 87,	
		 88,	 89,	 90,	 91,	 92,	 93,	 94,	 95,
		 96,	 97,	 98,	 99,	100,	101,	102,	103,
		104,	105,	106,	107,	108,	109,	110,	111,
		112,	113,	114,	115,	116,	117,	118,	119,
		120,	121,	122,	123,	124,	126,	128,	130,	
		132,	134,	136,	138,	140,	142,	144,	146,
		148,	150,	152,	154,	156,	158,	160,	162,
		164,	166,	168,	170,	172,	174,	176,	178,
		181,	184,	187,	190,	193,	196,	199,	202,
		205,	208,	211,	214,	217,	220,	223,	226,
		229,	232,	236,	240,	244,	248,	252,	256,
		260,	264,	268,	272,	276,	280,	284,	289,
		294,	299,	304,	309,	314,	319,	324,	329,
		334,	339,	345,	351,	357,	363,	369,	375,
		381,	387,	394,	401,	408,	415,	422,	429,
		436,	443,	451,	459,	467,	475,	483,	491,
		500,	509,	518,	527,	536,	546,	556,	566,
		576,	586,	597,	608,	619,	630,	641,	653,
		665,	677,	689,	702,	715,	728,	741,	755,
		769,	783,	798,	813,	828,	844,	860,	876,
		893,	910,	928,	946,	964,	983,	1003,	1024};
void SetLampIntensity (u8 ubLampIntensitySetting)
{
	EvbRegs.CMPR5 = uIntensityTable[ubLampIntensitySetting];			
}
#else
	// Logarithmically spaced from 1 to 254. 
	// Formula:  Intensity[i] = round(32768* 10^((i-254)/253*3))
	// Input  1 -->   0.100% 
	//        2 -->   0.103%
	// 		  3 -->   0.106%
	//		. . .
	//		252	-->	 94.686%
	//		253 -->	 97.307%
	//		254 --> 100.000%
	// 		255 --> 100.000%
	u16	uIntensityTable[256] = 
	  {	 0,  	33,  	34,  	35, 	36, 	37, 	38, 	39,
		40,  	41,  	42,  	43, 	44, 	45, 	47, 	48,
		49,  	51,  	52, 	54, 	55, 	57, 	58, 	60,
		61,  	63,  	65, 	67, 	68, 	70, 	72, 	74,
		76,  	79,  	81, 	83, 	85, 	88, 	90, 	92,
		95,  	98,  	100,	103,	106,	109,	112,	115,
		118,	122,	125,	128,	132,	136,	139,	143,
		147,	151,	155,	160,	164,	169,	173,	178,
		183,	188,	193,	199,	204,	210,	216,	222,
		228,	234,	240,	247,	254,	261,	268,	276,
		283,	291,	299,	307,	316,	325,	334,	343,
		352,	362,	372,	383,	393,	404,	415,	427,
		438,	451,	463,	476,	489,	503,	517,	531,
		545,	561,	576,	592,	608,	625,	643,	660,
		679,	697,	717,	737,	757,	778,	799,	822,
		844,	868,	892,	916,	942,	968,	995,	1022,
		1050,	1080,	1109,	1140,	1172,	1204,	1237,	1272,
		1307,	1343,	1380,	1418,	1458,	1498,	1540,	1582,
		1626,	1671,	1717,	1765,	1814,	1864,	1915,	1968,
		2023,	2079,	2136,	2196,	2256,	2319,	2383,	2449,
		2517,	2586,	2658,	2731,	2807,	2885,	2965,	3047,
		3131,	3218,	3307,	3398,	3492,	3589,	3688,	3790,
		3895,	4003,	4114,	4228,	4345,	4465,	4589,	4716,
		4846,	4980,	5118,	5260,	5406,	5555,	5709,	5867,
		6029,	6196,	6368,	6544,	6725,	6911,	7103,	7299,
		7501,	7709,	7922,	8142,	8367,	8598,	8836,	9081,
		9332,	9591,	9856,	10129,	10409,	10698,	10994,	11298,
		11611,	11932,	12262,	12602,	12951,	13309,	13677,	14056,
		14445,	14845,	15256,	15678,	16112,	16558,	17016,	17487,
		17971,	18469,	18980,	19505,	20045,	20600,	21170,	21756,
		22358,	22977,	23613,	24267,	24939,	25629,	26338,	27067,
		27817,	28587,	29378,	30191,	31027,	31885,	32768,	32768};
void SetLampIntensity (u8 ubLampIntensitySetting)
{
#if DAC_SIZE < 15
  	u16			uOutVal;	// Intensity setting with full resolution
  	u16			uDACVal;	// Integer value to be sent to DAC.  Possibly less resolution than OutVal
 	static q16	qFrac = 0;	// Fractional part not sent
 	
 	uOutVal = uIntensityTable[ubLampIntensitySetting];
	uDACVal = (uOutVal+qFrac) >>(15-DAC_SIZE);			// Add fractional part, then truncate
	qFrac = (uOutVal+qFrac) - (uDACVal<<(15-DAC_SIZE));	// Determine fractional part
	EvbRegs.CMPR5 = uDACVal;
#else	
	EvbRegs.CMPR5 = uIntensityTable[ubLampIntensitySetting];			
#endif
}
 #endif

