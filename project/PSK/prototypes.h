//==========================================================================================
// Filename:		prototypes.h
//
//
// Description:		Header files with global prototypes.
//
// Copyright (C) 2002 Texas Instruments Incorporated
// Texas Instruments Proprietary Information
// Use subject to terms and conditions of TI Software License Agreement
//
// Revision History:
// 06/20/02	EGO		Started file.
// 03/13/03	HEM		Stripped out unused stuff from TEC project.
// 11/17/04	HEM		Stripped out unused stuff from CAN project.
//==========================================================================================



#ifndef prototype_h						// Header file guard
#define	prototype_h


// Include prototypes for functions defined in DSP28_xxx.c files.
#include "DSP28_GlobalPrototypes.h"
#include "main.h"

// command.c
extern void TaskCommand(void);

// dacout.c
extern void ConfigurePWMDAC(void);
extern void WriteAuxDAC(u16 uChan, q16 qOutVal);
extern void WritePWMDAC(u16 uChan, q16 qOutVal);
extern void SetLampIntensity (u8 ubLampIntensitySetting);
extern void ControlLamp(void);
extern void InitLampVars(void);

// diag.c
//extern void InitDiagTrace(void);
//extern u16 CmdDiagTraceConfig(void);
//extern u16 RunTrace(void);

// gpio.c
extern void InitGpio(void);

// main.c
extern interrupt void ISRTimer0(void);

// subs.asm
u16 ReadProg(u16 Address);
void WriteProg(u16 Address, u16 Data);
extern u16 SmoothADCResults (void);	

// fir16.asm
extern q32	FIR2(q16 ADCResult);
extern q32	AutoCorr(void);

// transmit.c
extern void FillTxBuffer(u16 uUserTxMsgLen);
extern u16 GenerateFakePLCMessage(u16	uSeed);
extern u16 GenerateFloodPLCMessage(u16	uSeed);

// detData.c
extern void ProcessRxPlcMsg(void);
extern void receive(s16 ADCsample);
extern void reset_to_BitSync(void);

// crc.c
void InitCRCtable(void);
void AppendParityCheckBytes(u16 *pUserData, u16 numWords);
u16 CompareParityCheckBytes(u16 *pUserData, u16 numBytes);
u16 CalcCRC(u16 uPlcMode, u16 numBytes);

// uart.c
extern void InitSci(void);
extern void InitializeUARTArray(void);
extern void HandleUART (void);
extern u16 WriteUART(u16 uCount, u16* upData);
extern u16 WriteUARTValue(u16 uValue);

// sensor.c
extern void	ConfigureADCs(void);
extern void ReadAllSensors(u16 uWaitFlag);
extern u16 SmoothSensor(void);
extern void ArmAllSensors(void);


// timer.c
extern void ConfigureGPTimers(void);
extern void DelayNms(u16 uN);
extern void DelayNus(u16 uN);
extern void FeedDog(void);

// vardefs.c
extern void InitializeGlobals(void);

//diag2.c
extern void SaveTraceF(u16 parm);
extern void SaveTraceInit(u16 length);

//==========================================================================================
#endif									// End of header guard: #ifndef prototype_h



