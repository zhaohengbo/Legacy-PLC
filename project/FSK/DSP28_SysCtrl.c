//
//      TMDX ALPHA RELEASE
//      Intended for product evaluation purposes
//
//###########################################################################
//
// FILE:	DSP28_SysCtrl.c
//
// TITLE:	DSP28 Device System Control Initialization & Support Functions.
//
//###########################################################################
//
//  Ver | dd mmm yyyy | Who  | Description of changes
// =====|=============|======|===============================================
//  0.55| 06 May 2002 | L.H. | EzDSP Alpha Release
//  0.56| 20 May 2002 | L.H. | No change
//  0.57| 24 May 2002 | L.H. | Added initialization of RAM control registers
//      |             |      | for TMX samples.
//      | 08 Aug 2002 | M.H. | Enable MCBSP clock.
//      | 12 Sep 2002 | M.H. | Put NOP inside wait loop so optimizer doesn't
//      |             |      | eliminate the wait.
//  0.58| 29 Jun 2002 | L.H. | Added InitFlash function.  Must be run from RAM
//      | 17 Oct 2002 | HEM  | Enabled CAN clock.
//      | 04 Feb 2003 | HEM  | Configure DSP for 120 MIPS option.
//      | 06 Feb 2003 | HEM  | Slow down watchdog so it doesn't expire before
//      |             |      |  we get done with initialization.
//      | 26 Mar 2003 | HEM  | Set WDOVERRIDE to allow us to shut off watchdog.
//      | 08 Oct 2004 | HEM  | Set flash wait states based on clock speed.
//###########################################################################

#include "main.h"
#include "DSP28_Device.h"

// Functions that will be run from RAM need to be assigned to 
// a different section.  This section will then be mapped using
// the linker cmd file.
#ifdef __cplusplus			// "C++"
#pragma CODE_SECTION("ramfuncs");
#else						// "C"
//#pragma CODE_SECTION(InitSysCtrl, "ramfuncs");
#pragma CODE_SECTION(InitFlash, "ramfuncs");
#endif

//---------------------------------------------------------------------------
// InitSysCtrl: 
//---------------------------------------------------------------------------
// This function initializes the System Control registers to a known state.
//
void InitSysCtrl(void)
{
   Uint16 i;
   EALLOW;
   
// On TMX samples, to get the best performance of on chip RAM blocks M0/M1/L0/L1/H0 internal
// control registers bit have to be enabled. The bits are in Device emulation registers.
   DevEmuRegs.M0RAMDFT = 0x0300;
   DevEmuRegs.M1RAMDFT = 0x0300;
   DevEmuRegs.L0RAMDFT = 0x0300;
   DevEmuRegs.L1RAMDFT = 0x0300;
   DevEmuRegs.H0RAMDFT = 0x0300;
   
// Disable watchdog module
   SysCtrlRegs.SCSR.bit.WDENINT = 1;	// When watchdog expires, trigger an interrupt instead of resetting CPU.         
   SysCtrlRegs.WDCR= 0x0068 | 0x7;		// Disable watchdog, slow down dog timer to 1/64th speed


// Initalize PLL
#if (MIPS == 120)
   SysCtrlRegs.PLLCR = 8;		// 30 MHz * 8 / 2 = 120 MIPS
#else
   SysCtrlRegs.PLLCR = 10;		// 30 MHz * 10 / 2 = 150 MIPS
#endif

   // Wait for PLL to lock
   for(i= 0; i< 5000; i++)
   {
   	NOP;
   }
       
// HISPCP/LOSPCP prescale register settings, normally it will be set to default values
   SysCtrlRegs.HISPCP.all = 0x0000; 	// Run Hi-speed periph clock at max speed  

   SysCtrlRegs.LOSPCP.all = 0x0002;		// Run Lo-speed periph clock at SYSCLKOUT/4 (150/4=37.5 MHz)
// Peripheral clock enables set for the selected peripherals.   
   SysCtrlRegs.PCLKCR.bit.EVAENCLK=1;
   SysCtrlRegs.PCLKCR.bit.EVBENCLK=1;
   SysCtrlRegs.PCLKCR.bit.ADCENCLK=1;
   SysCtrlRegs.PCLKCR.bit.SPIENCLK=0;		//Off
   SysCtrlRegs.PCLKCR.bit.SCIENCLKA=1;
   SysCtrlRegs.PCLKCR.bit.SCIENCLKB=0;		//Off
   SysCtrlRegs.PCLKCR.bit.MCBSPENCLK=1;
   SysCtrlRegs.PCLKCR.bit.ECANENCLK=1;	
				
   EDIS;
	
}

// This function initializes the Flash Control registers

//                   CAUTION 
// This function MUST be executed out of RAM. Executing it
// out of OTP/Flash will yield unpredictable results

void InitFlash(void)
{
   EALLOW;
   //Enable Flash Pipeline mode to improve performance
   //of code executed from Flash.
   FlashRegs.FOPT.bit.ENPIPE = 1;
   
   //                CAUTION
   // Minimum waitstates required for the flash operating
   // at a given CPU rate must be characterized by TI. 
   // Refer to the datasheet SPRS174 for the latest information.  

#if (MIPS <= 25)	
   FlashRegs.FBANKWAIT.bit.RANDWAIT = 1;	//Set the Random Waitstate for the Flash
   FlashRegs.FBANKWAIT.bit.PAGEWAIT = 0;	//Set the Paged Waitstate for the Flash
#elif (MIPS <= 50)	
   FlashRegs.FBANKWAIT.bit.RANDWAIT = 1;	//Set the Random Waitstate for the Flash
   FlashRegs.FBANKWAIT.bit.PAGEWAIT = 1;	//Set the Paged Waitstate for the Flash
#elif (MIPS <= 75)	
   FlashRegs.FBANKWAIT.bit.RANDWAIT = 2;	//Set the Random Waitstate for the Flash
   FlashRegs.FBANKWAIT.bit.PAGEWAIT = 2;	//Set the Paged Waitstate for the Flash
#elif (MIPS <= 100)	
   FlashRegs.FBANKWAIT.bit.RANDWAIT = 3;	//Set the Random Waitstate for the Flash
   FlashRegs.FBANKWAIT.bit.PAGEWAIT = 3;	//Set the Paged Waitstate for the Flash
#elif (MIPS <= 75)	
   FlashRegs.FBANKWAIT.bit.RANDWAIT = 2;	//Set the Random Waitstate for the Flash
   FlashRegs.FBANKWAIT.bit.PAGEWAIT = 2;	//Set the Paged Waitstate for the Flash   
#elif (MIPS <= 120)	
   FlashRegs.FBANKWAIT.bit.RANDWAIT = 4;	//Set the Random Waitstate for the Flash
   FlashRegs.FBANKWAIT.bit.PAGEWAIT = 4;	//Set the Paged Waitstate for the Flash   
#else   
   FlashRegs.FBANKWAIT.bit.RANDWAIT = 5;	//Set the Random Waitstate for the Flash
   FlashRegs.FBANKWAIT.bit.PAGEWAIT = 5;	//Set the Paged Waitstate for the Flash     
#endif   
   
   //                CAUTION
   //Minimum cycles required to move between power states
   //at a given CPU rate must be characterized by TI. 
   //Refer to the datasheet for the latest information.
     
   //For now use the default count
   
   //Set number of cycles to transition from sleep to standby
   FlashRegs.FSTDBYWAIT.bit.STDBYWAIT = 0x01FF;       
   
   //Set number of cycles to transition from standby to active
   FlashRegs.FACTIVEWAIT.bit.ACTIVEWAIT = 0x01FF;   
   EDIS;
}	


//---------------------------------------------------------------------------
// KickDog: 
//---------------------------------------------------------------------------
// This function resets the watchdog timer.
// Enable this function for using KickDog in the application 
/*
void KickDog(void)
{
    EALLOW;
    SysCtrlRegs.WDKEY = 0x0055;
    SysCtrlRegs.WDKEY = 0x00AA;
    EDIS;
}
*/	
	
//===========================================================================
// No more.
//===========================================================================
