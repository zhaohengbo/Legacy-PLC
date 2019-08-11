//==========================================================================================
// Filename:		main.c
//
// Description:		Main file.  Contains main() function which sets up the DSP and all
//					required peripherals, then drops into an infinite loop which is
//					executed once per period.
//
// Copyright (C) 2002 Texas Instruments Incorporated
// Texas Instruments Proprietary Information
// Use subject to terms and conditions of TI Software License Agreement
//
// Revision History:
// 05/08/02	EGO		Started file.
// 06/20/02 EGO		Removed unnecessay includes.
//					Moved ISRTimer2() back here from cisr.c for clarity.
// 06/21/02 EGO		Removed vardefs.h.  Function now performed by call to InitializeGlobals().
// 10/15/02 HEM		Changed from timer2 to timer0.
// 02/07/03 HEM/KKN Added BootCopy routine.
//==========================================================================================

#include "main.h"          

void MainLoop(void);
 
#define	FLOOD_SEED_SHIFT	16					// Flood message interval shift (power of 2)

u32 ulLampTimeStamp = 0;	
u32 ulFloodTimeStamp = 0;
	
	
#ifdef _Release
	// Functions that will be run from RAM need to be assigned to 
	// a different section.  This section will then be mapped using
	// the linker cmd file.
	#ifdef __cplusplus			// "C++"
	#pragma CODE_SECTION("ramfuncs"); 
	#else						// "C"
	#pragma CODE_SECTION(ISRTimer0, "ramfuncs");
	#pragma CODE_SECTION(MainLoop, "ramfuncs");
	#endif
	
	
	//==========================================================================================
	// Function:		BootCopy()
	//
	// Description: 	Copies the InitFlash function from FLASH into RAM so that it can
	//					adjust the Wait States of the flash.  (Don't try to adjust Flash
	//					wait states while running from flash)  Only in RELEASE code.
	//
	// Revision History:
	// 02/07/03	KKN		Started function.  FAR statements needed if using SMALL MEMORY model, otherwise
	//					pointer values get truncated to 16-bit.
	//==========================================================================================
	void BootCopy(void)
	{
		// Information on the location of functions that are going
		// to be relocated to RAM
		#define RAM_FUNC_LOAD   0x3F4000    // Source location in Flash
		#define RAM_FUNC_LENGTH 0x000800    // Number of 32-bit values to copy
		#define RAM_FUNC_RUN    0x3F8000    // Destination location in RAM
		
		Uint32  far *pSourceAddr;
	    Uint32  far *pDestAddr;
	    Uint16 i; 
		pSourceAddr = (Uint32 far *)RAM_FUNC_LOAD;
		pDestAddr = (Uint32 far *)RAM_FUNC_RUN;
		for(i = 0; i < RAM_FUNC_LENGTH; i++)
		{
		    *pDestAddr++ = *pSourceAddr++;
		}
	}
#endif


//==========================================================================================
// Function:		ElapsedTime()
//
// Description: 	Returns elapsed time between two events.
//
// Revision History:
// 09/23/04	HEM		New fucntion.
//==========================================================================================
inline u32	ElapsedTime(u32 ulStopTime, u32 ulStartTime)
{
	if (ulStopTime > ulStartTime)
	{
		return (ulStopTime - ulStartTime);				// Normal
	}
	else
	{
		return ((ulStopTime - 0x80000000) - (ulStartTime + 0x80000000));	// Rollover
	}
}


//==========================================================================================
// Function:		ISRTimer0()
//
// Description: 	This timer is used to implement the main system periodic timer.
//
// Revision History:
// 06/17/02 EGO		Started function with cut and paste from Example_28xDevice.c
// 10/15/02 HEM		Changed from timer2 to timer0.
// 10/17/02 KKN		Acknowledge the PIE interrupt to re-enable interrupts
// 03/12/03 HEM		Re-enable interrupts before returning to baseline.
//==========================================================================================
interrupt void ISRTimer0(void)
{
	CpuTimer0.InterruptCount++;
	PieCtrlRegs.PIEACK.all = 0xFFFF;    // Enable PIE interrupts	
	EINT;   							// Re-Enable Global interrupt INTM
}


//==========================================================================================
// Function:		main()
//
// Description: 	Performs setup functions, then enters infinite loop to execute tasks.
//
// Revision History:
// 05/08/02	EGO		TEC: Started function.
// 03/20/03	HEM		CAN: Stripped out unused stuff from TEC project.
// 07/--/04	HEM		PLC: Started from CAN project.
//					Removed CAN functions.
//					Added interrupt configuration for ADC and Event Manager.
//					Generate fake messages periodically.
// 10/09/04	HEM		Break MainLoop() into its own function so we can execute it from RAM
//					for best speed.  We cannot move main() to RAM because main() needs to 
//					call the BootCopy before anything can execute from RAM.
//==========================================================================================
#ifdef __cplusplus
int main(void)
#else
void main(void)
#endif
{
	// Disable and clear all CPU interrupts:
	DINT;
	IER = 0x0000; 
	IFR = 0x0000; 

	// Initialize System Control registers, PLL, WatchDog, Clocks to default state:
	InitSysCtrl();

	// Select GPIO for the device or for the specific application:
	InitGpio();

	// Initialize Pie Control Registers To Default State and populate the PIE vector table with
	// pointers to the shell Interrupt Service Routines (ISR)functions found in DSP28_DefaultIsr.c.
	InitPieCtrl();			// Initialize PIE control registers
	InitPieVectTable();		// Populate the PIE vector table with default pointers


#ifdef _Release		// Initialize FLASH on Release version of code
	BootCopy();		// Copy the InitFlash routine to RAM
	InitFlash();	// Set up flash waitstates (This function >MUST< reside in RAM)
#endif

	
	InitCRCtable();	// Initialize the CRC table

	// Initialize all peripherals (EV's, ADC, SPI, SCI's, CAN, McBSP, CPU Timer and XIntf) to default settings
	#if  DSP_TYPE == 2812
		InitXintf();
	#endif 
	InitCpuTimers(); 
	InitSci(); 
	ConfigureADCs();
	NOP;
	
	ConfigureGPTimers();	// Sets up GP timers for the PWM and ADC
//	ConfigurePWMDAC();		// Configures the PWM D/A converters

	// Software initialization
 	InitializeGlobals();	// Global variable initialization.
	InitializeUARTArray();	// Clear out array of data to be sent to UART.

	uMyAddress =  0x0100 |	(INPUT_B9 << 1) | (INPUT_B10<<0);	// Set my address based on GPIO jumper settings
	//                       ^ OPT2 Jumper     ^ OPT1 Jumper
	
	reset_to_BitSync();		// reset digital PLL states

	
	InitLampVars();			// 
	
	// Configure DSP Timer 0 to drive the periodic interrupt loop
	// 		Set Up For 0.5 millisecond Interrupt Period
	// 		Point PIE vector to "ISRTimer0" function
	// 		Connect To INT14
	//		Restart the interrupt counter
	
	// Reassign the PIE vector for TINT0 to use the ISRTimer0() routine instead of the default ISR in DSP28_DefaultIsr.c.
	EALLOW;								// This is needed to write to EALLOW protected registers
	PieVectTable.TINT0 = &ISRTimer0;	// Point the PIE vector for TINT0 to the ISRTimer0() routine
	PieVectTable.ADCINT = &ADCINT_ISR;	// Point the PIE vector for ADC INT to the ADCINT_ISR() routine,
	EDIS;   							// This is needed to disable write to EALLOW protected registers
	
	// Configure CPU-Timer 0 to interrupt every millisecond:
	ConfigCpuTimer(&CpuTimer0, MIPS, 1000000/TINTS_PER_SEC);	// CPU Freq, Period (in uSeconds)
 	StartCpuTimer0();

    // Enable INT1 which is connected to CPUTimer0 tru PIE 1.7 and ADC thru PIE1.6
	PieCtrlRegs.PIEIER1.bit.INTx6 = 1;		// PIE 1.6  ADC Interrrupt  
	PieCtrlRegs.PIEIER1.bit.INTx7 = 1;		// PIE 1.7	CPU Timer0 Interrupt		
	IER |= M_INT1;

	EALLOW;								// This is needed to write to EALLOW protected registers	
	PieVectTable.T1PINT = &T1PINT_ISR;	// Point the PIE vector for T1PINT to the T1PINT_ISR() routine
	EDIS;   							// This is needed to disable write to EALLOW protected registers				
	
	//TX  enable these lines to use a separate interupt for the transmit timing
	//TX	// Enable INT2 which is connected to Event Manager A Timer 1 Thru PIE 2.4
	//TX	PieCtrlRegs.PIEIER2.bit.INTx4 = 1;		// PIE 2.4	Event Manager A Timer 1
	//TX	IER |= M_INT2;	

	ArmAllSensors();		// Arm the sensors for the next reading
	
    // Enable global Interrupts and higher priority real-time debug events:
	IFR = 0x0000;	// Clear any pending interrupts
	PieCtrlRegs.PIEACK.all = 0xFFFF;    // Enable PIE interrupts	
	ERTM;			// Enable Global realtime interrupt DBGM

	// The setup of the system is complete at this point.  This loop replaces BIOS.
	
	MainLoop();		// MainLoop runs forever
}


//==========================================================================================
// Function:		MainLoop()
//
// Description: 	Infinite loop to execute tasks.
//
// Revision History:
// 10/09/04	HEM		Broke MainLoop() into its own function so we can execute it from RAM
//					for best speed.  We cannot move main() to RAM because main() needs to 
//					call the BootCopy before anything can execute from RAM.
// 11/09/04	HEM		Add task switching to spread processor load more evenly over time.
// 11/1x/04	HEM		Cleaned out lots of unused code. 
//					Comment out calls to FeedDog(), RunTrace(), and HandlePLC().
//					Wait in idle loop until ADCIntFlag is set.
// 11/19/04	HEM		Synchronized task switcher here with ADC Int by using ADCIntCount as selector.
//					Reduced from 25 cases down to 5.
// 23Feb05	Hagen	changed max ADC counter from 5 to 7 and redistributed tasks
//==========================================================================================
void	MainLoop(void)
{	
	u16	uTxMsgLen = COMMAND_PARMS;
	

	EINT;	// Enable Global interrupt INTM	
	
	for(;;)	// ============== T O P   O F   M A I N   L O O P =========================
	{
	
		switch (ADCIntCount)
		{
			case 0:
				break;

			case 1:
			{
				HandleUART();	// Handle incoming and outgoing serial port data.
								// Will always send and receive at least one char if they are ready
				NOP;			// NOP is not required, but it's a good spot for a break-point during debug.
				break;
			}
	
			case 2:
				break;

			case 3:
			{
				// uCommandActive indicates a command is being run or is ready to start.
				// Set by HandleUART() when a full command is ready.
				// Cleared by command functions when the command is finished.
				if (uCommandActive == 1)
				{
					TaskCommand(); 	// Handle the command
					uTxMsgLen = COMMAND_PARMS;
				}	
	
				NOP;			// NOP is not required, but it's a good spot for a break-point during debug.
				break;
			}
			
			case 4:
				break;

			case 5:
			{
				// Control lamp intensity
				if (ElapsedTime(CpuTimer0.InterruptCount, ulLampTimeStamp) >= uFadeInterval)
				{
					ulLampTimeStamp = CpuTimer0.InterruptCount;
					ControlLamp();
				}

			
				NOP;			// NOP is not required, but it's a good spot for a break-point during debug.
				break;
			}			
			
			case 6:
			{
				if ( (plcMode != TX_MODE) && 
					 (uTxMsgPending != True) &&
					 (uFloodInterval > 0) &&
				     (ElapsedTime(CpuTimer0.InterruptCount, ulFloodTimeStamp) >= uFloodInterval))
				{
					//uTxMsgLen = GenerateFakePLCMessage(ulTimerIntCounter>>FLOOD_SEED_SHIFT);
					uTxMsgLen = GenerateFloodPLCMessage(ulTimerIntCounter>>FLOOD_SEED_SHIFT);
					uTxMsgPending = True;
					ulFloodTimeStamp = CpuTimer0.InterruptCount;
				}
			
				NOP;			// NOP is not required, but it's a good spot for a break-point during debug.
				break;
			}
			
	
			default:
			{
				NOP;	
				break;
			}
		}


		// Send the pending transmit packet if there is no incoming packet in progress
		if (uRxMode == FIND_BITSYNC)
		{
			if (uTxMsgPending && (plcMode != TX_MODE))	
			{
				FillTxBuffer(uTxMsgLen);
				uTxMsgPending = ~True;
			}
		}
		

		uSampleNumber++;			// increment sample number for next pass.


		// === TOP OF  IDLE LOOP ===
		while (uADCIntFlag == 0)	// Wait here until ADC interrupt occurs
		{	

			ClearXF();				// Debug: Toggle XF to show idle time on external scope		
			EINT;   				//!!!DEBUG Enable Global interrupt INTM
			asm ("	IDLE");			// Put processor in low-power mode 
			// - - - I D L E - - -
			DINT;   				//!!!DEBUG Disable Global interrupt INTM
			NOP;					// Getting here means interrupt occurred and ISR is
									// finished. NOP is not required, but it's a good spot
									// for a break-point during debug.
		}
		// === BOTTOM OF  IDLE LOOP ===
		SetXF();					// Debug: Toggle XF to show idle time on external scope

		uADCIntFlag = 0;			// Clear the ADC interrupt flag

		if (uRxMsgPending)
		{
			ProcessRxPlcMsg();
		}

		ulTimerIntCounter++;		// Increment once per sample

	}	// ============== B O T T O M   O F   M A I N   L O O P =========================
}





