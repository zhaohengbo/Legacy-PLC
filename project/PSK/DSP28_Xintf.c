//
//      TMDX ALPHA RELEASE
//      Intended for product evaluation purposes
//
//###########################################################################
//
// FILE:	DSP28_Xintf.c
//
// TITLE:	DSP28 Device External Interface Init & Support Functions.
//
//###########################################################################
//
//  Ver | dd mmm yyyy | Who  | Description of changes
// =====|=============|======|===============================================
//  0.55| 06 May 2002 | L.H. | EzDSP Alpha Release
//  0.56| 20 May 2002 | L.H. | No change
//  0.57| 27 May 2002 | L.H. | No change
//      | 20 Jun 2002 | E.O. | Use flag DSP_TYPE, instead of just F2812 test
//  0.58| 29 Jun 2002 | L.H. | Updated the init function to the default state
//###########################################################################

#include "DSP28_Device.h"

//---------------------------------------------------------------------------
// InitXINTF: 
//---------------------------------------------------------------------------
// This function initializes the External Interface to a known state.
//
// Do not modify the timings of a zone while accessing that
// same zone.
void InitXintf(void)
{

	#if  DSP_TYPE == 2812
    // Setup the XINTF to the default state after reset
    
    // All Zones---------------------------------
    // Timing for all zones based on XTIMCLK = 1/2 SYSCLKOUT 
    XintfRegs.XINTCNF2.bit.XTIMCLK = 1;
    // No write buffering
    XintfRegs.XINTCNF2.bit.WRBUFF = 0;
    // XCLKOUT is enabled
    XintfRegs.XINTCNF2.bit.CLKOFF = 0;
    // XCLKOUT = XTIMCLK/4 
    XintfRegs.XINTCNF2.bit.CLKMODE = 1;
    
    
    // Zone 0------------------------------------
	// When using ready, ACTIVE must be 1 or greater
	// Lead must always be 1 or greater
	// Zone write timing
	XintfRegs.XTIMING0.bit.XWRLEAD = 3;
	XintfRegs.XTIMING0.bit.XWRACTIVE = 7;
	XintfRegs.XTIMING0.bit.XWRTRAIL = 3;
    // Zone read timing
    XintfRegs.XTIMING0.bit.XRDLEAD = 3;
    XintfRegs.XTIMING0.bit.XRDACTIVE = 7;
    XintfRegs.XTIMING0.bit.XRDTRAIL = 3;
    
	// double all Zone read/write lead/active/trail timing 
	XintfRegs.XTIMING0.bit.X2TIMING = 1;
	
	// Zone will sample XREADY signal 
	XintfRegs.XTIMING0.bit.USEREADY = 1;
	XintfRegs.XTIMING0.bit.READYMODE = 1;  // sample asynchronous
	
	// Size must be 1,1 - other values are reserved
	XintfRegs.XTIMING0.bit.XSIZE = 3;
    
    // Zone 1------------------------------------
	// When using ready, ACTIVE must be 1 or greater
	// Lead must always be 1 or greater
	// Zone write timing
	XintfRegs.XTIMING1.bit.XWRLEAD = 3;
	XintfRegs.XTIMING1.bit.XWRACTIVE = 7;
	XintfRegs.XTIMING1.bit.XWRTRAIL = 3;
    // Zone read timing
    XintfRegs.XTIMING1.bit.XRDLEAD = 3;
    XintfRegs.XTIMING1.bit.XRDACTIVE = 7;
    XintfRegs.XTIMING1.bit.XRDTRAIL = 3;
    
	// double all Zone read/write lead/active/trail timing 
	XintfRegs.XTIMING1.bit.X2TIMING = 1;
	
	// Zone will sample XREADY signal 
	XintfRegs.XTIMING1.bit.USEREADY = 1;
	XintfRegs.XTIMING1.bit.READYMODE = 1;  // sample asynchronous
	
	// Size must be 1,1 - other values are reserved
	XintfRegs.XTIMING1.bit.XSIZE = 3;

    // Zone 2------------------------------------
	// When using ready, ACTIVE must be 1 or greater
	// Lead must always be 1 or greater
	// Zone write timing
	XintfRegs.XTIMING2.bit.XWRLEAD = 3;
	XintfRegs.XTIMING2.bit.XWRACTIVE = 7;
	XintfRegs.XTIMING2.bit.XWRTRAIL = 3;
    // Zone read timing
    XintfRegs.XTIMING2.bit.XRDLEAD = 3;
    XintfRegs.XTIMING2.bit.XRDACTIVE = 7;
    XintfRegs.XTIMING2.bit.XRDTRAIL = 3;
    
	// double all Zone read/write lead/active/trail timing 
	XintfRegs.XTIMING2.bit.X2TIMING = 1;
	
	// Zone will sample XREADY signal 
	XintfRegs.XTIMING2.bit.USEREADY = 1;
	XintfRegs.XTIMING2.bit.READYMODE = 1;  // sample asynchronous
	
	// Size must be 1,1 - other values are reserved
	XintfRegs.XTIMING2.bit.XSIZE = 3;


    // Zone 6------------------------------------
	// When using ready, ACTIVE must be 1 or greater
	// Lead must always be 1 or greater
	// Zone write timing
	XintfRegs.XTIMING6.bit.XWRLEAD = 3;
	XintfRegs.XTIMING6.bit.XWRACTIVE = 7;
	XintfRegs.XTIMING6.bit.XWRTRAIL = 3;
    // Zone read timing
    XintfRegs.XTIMING6.bit.XRDLEAD = 3;
    XintfRegs.XTIMING6.bit.XRDACTIVE = 7;
    XintfRegs.XTIMING6.bit.XRDTRAIL = 3;
    
	// double all Zone read/write lead/active/trail timing 
	XintfRegs.XTIMING6.bit.X2TIMING = 1;
	
	// Zone will sample XREADY signal 
	XintfRegs.XTIMING6.bit.USEREADY = 1;
	XintfRegs.XTIMING6.bit.READYMODE = 1;  // sample asynchronous
	
	// Size must be 1,1 - other values are reserved
	XintfRegs.XTIMING6.bit.XSIZE = 3;


    // Zone 7------------------------------------
	// When using ready, ACTIVE must be 1 or greater
	// Lead must always be 1 or greater
	// Zone write timing
	XintfRegs.XTIMING7.bit.XWRLEAD = 3;
	XintfRegs.XTIMING7.bit.XWRACTIVE = 7;
	XintfRegs.XTIMING7.bit.XWRTRAIL = 3;
    // Zone read timing
    XintfRegs.XTIMING7.bit.XRDLEAD = 3;
    XintfRegs.XTIMING7.bit.XRDACTIVE = 7;
    XintfRegs.XTIMING7.bit.XRDTRAIL = 3;
    
	// double all Zone read/write lead/active/trail timing 
	XintfRegs.XTIMING7.bit.X2TIMING = 1;
	
	// Zone will sample XREADY signal 
	XintfRegs.XTIMING7.bit.USEREADY = 1;
	XintfRegs.XTIMING7.bit.READYMODE = 1;  // sample asynchronous
	
	// Size must be 1,1 - other values are reserved
	XintfRegs.XTIMING7.bit.XSIZE = 3;

    // Bank switching
    // Assume Zone 7 is slow, so add additional BCYC cycles 
    // when ever switching from Zone 7 to another Zone.  
    // This will help avoid bus contention.
    XintfRegs.XBANK.bit.BANK = 7;
    XintfRegs.XBANK.bit.BCYC = 7; 
    
	#endif
}	
	
//===========================================================================
// No more.
//===========================================================================
