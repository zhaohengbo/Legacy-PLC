//###########################################################################
//
// FILE:	Example.h
//
// TITLE:	Example program definition file 
//###########################################################################
//
//  Ver | dd mmm yyyy | Who  | Description of changes
// =====|=============|======|===============================================
//  0.55| 06 May 2002 | L.H. | EzDSP Alpha Release
//  0.56| 20 May 2002 | L.H. | No change
//  0.57| 27 May 2002 | L.H. | No change
//      | 20 Jun 2002 | E.O. | Commented out unused prototypes.
//  0.58| 29 Jun 2002 | L.H. | Added InitFlash
//###########################################################################


#ifndef EXAMPLE_H
#define EXAMPLE_H

/*---- shared global function prototypes -----------------------------------*/
//extern void InitAdc(void);
//extern void InitDevEmu(void);
//extern void InitDevice(void);
extern void InitECan(void);
//extern void InitEv(void);
//extern void InitGpio(void);
//extern void InitMcbsp(void);
extern void InitPieCtrl(void);
extern void InitPieVectTable(void);
//extern void InitSci(void);
//extern void InitSpi(void);
extern void InitSysCtrl(void);
extern void InitXintf(void);
extern void InitECan(void);
//extern void InitXIntrupt(void);

//                 CAUTION
// This function MUST be executed out of RAM. Executing it
// out of OTP/Flash will yield unpredictable results
extern void InitFlash(void);
//extern void KickDog(void);

#endif   // - end of EXAMPLE_H

