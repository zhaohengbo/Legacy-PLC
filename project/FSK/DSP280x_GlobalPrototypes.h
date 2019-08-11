// TI File $Revision: /main/2 $
// Checkin $Date: December 2, 2004   17:46:03 $
//###########################################################################
//
// FILE:   DSP280x_GlobalPrototypes.h
//
// TITLE:  Global prototypes for DSP280x Examples
// 
//###########################################################################
// $TI Release: DSP280x V1.00 $
// $Release Date: December 9, 2004 $
//###########################################################################

#ifndef DSP280X_GLOBALPROTOTYPES_H
#define DSP280X_GLOBALPROTOTYPES_H


#ifdef __cplusplus
extern "C" {
#endif

/*---- shared global function prototypes -----------------------------------*/
extern void InitAdc(void);
extern void InitPeripherals(void);
extern void InitECan(void);
extern void InitECanGpio(void);
extern void InitECanaGpio(void);
#if DSP28_2808
extern void InitECanbGpio(void);
#endif // if DSP28_2808

extern void InitECap(void);
extern void InitECapGpio(void);
extern void InitECap1Gpio(void);
extern void InitECap2Gpio(void);
#if DSP28_2808 || DSP28_2806 
extern void InitECap3Gpio(void);
extern void InitECap4Gpio(void);
#endif // if DSP28_2808 || DSP28_2806 

extern void InitEPwm(void);
extern void InitEPwmGpio(void);
extern void InitEPwm1Gpio(void);
extern void InitEPwm2Gpio(void);
extern void InitEPwm3Gpio(void);
#if DSP28_2808 || DSP28_2806 
extern void InitEPwm4Gpio(void);
extern void InitEPwm5Gpio(void);
extern void InitEPwm6Gpio(void);
#endif // if DSP28_2808 || DSP28_2806 

extern void InitEQep(void);
extern void InitEQepGpio(void);
extern void InitEQep1Gpio(void);
#if DSP28_2808 || DSP28_2806 
extern void InitEQep2Gpio(void);
#endif // if DSP28_2808 || DSP28_2806 


extern void InitGpio(void);

extern void InitI2CGpio(void);

extern void InitPieCtrl(void);
extern void InitPieVectTable(void);

extern void InitSci(void);
extern void InitSciGpio(void);
extern void InitSciaGpio(void);
#if DSP28_2808 || DSP28_2806 
extern void InitScibGpio(void);
#endif // if DSP28_2808 || DSP28_2806 

extern void InitSpi(void);
extern void InitSpiGpio(void);
extern void InitSpiaGpio(void);
extern void InitSpibGpio(void);
#if DSP28_2808 || DSP28_2806
extern void InitSpicGpio(void);
extern void InitSpidGpio(void);
#endif

extern void InitSysCtrl(void);
extern void InitTzGpio(void);
extern void InitXIntrupt(void);
extern void InitPll(Uint16 val);
extern void InitPeripheralClocks(void);
extern void EnableInterrupts(void);
extern void DSP28x_usDelay(Uint32 Count);




// Watchdog functions
// DSP28_SysCtrl.c
extern void KickDog(void);
extern void DisableDog(void);

// DSP28_DBGIER.asm
extern void SetDBGIER(Uint16 dbgier);




//                 CAUTION
// This function MUST be executed out of RAM. Executing it
// out of OTP/Flash will yield unpredictable results
extern void InitFlash(void);


void MemCopy(Uint16 *SourceAddr, Uint16* SourceEndAddr, Uint16* DestAddr);


//---------------------------------------------------------------------------
// External symbols created by the linker cmd file
// DSP28 examples will use these to relocate code from one LOAD location 
// in either Flash or XINTF to a different RUN location in internal
// RAM
extern Uint16 RamfuncsLoadStart;
extern Uint16 RamfuncsLoadEnd;
extern Uint16 RamfuncsRunStart;

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif   // - end of DSP280X_GLOBALPROTOTYPES_H

//===========================================================================
// End of file.
//===========================================================================
