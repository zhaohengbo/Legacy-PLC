//
//      TMDX ALPHA RELEASE
//      Intended for product evaluation purposes
//
//###########################################################################
//
// FILE:	DSP28_GlobalVariableDefs.c
//
// TITLE:	DSP28 Global Variables and Data Section Pragmas.
//
//###########################################################################
//
//  Ver | dd mmm yyyy | Who  | Description of changes
// =====|=============|======|===============================================
//  0.55| 06 May 2002 | L.H. | EzDSP Alpha Release
//  0.56| 21 May 2002 | L.H. | Corrected case typo - PIE -> Pie
//  0.57| 27 May 2002 | L.H. | No change
//      | 06 Aug 2002 | M.H. | Fixed typo McbspaRegs -> McbspRegs
//  0.58| 18 Jun 2002 | L.H. | Changed PieCtrl to PieCtrlRegs 
//      | 20 Dec 2002 | M.H. | Added eCAN sections for MOTS and LAMs.
//      | 31 Jan 2003 | M.H. | Added C++ version of PRAGMAs
//      | 15 Feb 2005 | Hag  | commeted out mailbox RAM for use as PLC CRC table
//###########################################################################



//---------------------------------------------------------------------------
// Define Global Peripheral Variables:
//

#include "DSP28_Device.h"

#ifdef __cplusplus			// "C++"

#pragma DATA_SECTION("AdcRegsFile");
volatile struct ADC_REGS AdcRegs;

#pragma DATA_SECTION("CpuTimer0RegsFile");
volatile struct CPUTIMER_REGS CpuTimer0Regs;

#pragma DATA_SECTION("CpuTimer1RegsFile");
volatile struct CPUTIMER_REGS CpuTimer1Regs;

#pragma DATA_SECTION("CpuTimer2RegsFile");
volatile struct CPUTIMER_REGS CpuTimer2Regs;

#pragma DATA_SECTION("ECanaRegsFile");
volatile struct ECAN_REGS ECanaRegs;

#pragma DATA_SECTION("ECanaRegsFile");
volatile struct LAM_REGS ECanaLAMRegs;

#pragma DATA_SECTION("ECanaRegsFile");
volatile struct MOTS_REGS ECanaMOTSRegs;

#pragma DATA_SECTION("ECanaRegsFile");
volatile struct MOTO_REGS ECanaMOTORegs;

#pragma DATA_SECTION("ECanaMboxesFile");
volatile struct ECAN_MBOXES ECanaMboxes;

#pragma DATA_SECTION("EvaRegsFile");
volatile struct EVA_REGS EvaRegs;

#pragma DATA_SECTION("EvbRegsFile");
volatile struct EVB_REGS EvbRegs;

#pragma DATA_SECTION("GpioDataRegsFile");
volatile struct GPIO_DATA_REGS GpioDataRegs;

#pragma DATA_SECTION("GpioMuxRegsFile");
volatile struct GPIO_MUX_REGS GpioMuxRegs;

#pragma DATA_SECTION("McbspRegsFile");
volatile struct MCBSP_REGS McbspRegs;

#pragma DATA_SECTION("PieCtrlRegsFile");
volatile struct PIE_CTRL_REGS PieCtrlRegs;

#pragma DATA_SECTION("PieVectTable");
struct PIE_VECT_TABLE PieVectTable;

#pragma DATA_SECTION("SciaRegsFile");
volatile struct SCI_REGS SciaRegs;

#pragma DATA_SECTION("ScibRegsFile");
volatile struct SCI_REGS ScibRegs;

#pragma DATA_SECTION("SpiaRegsFile");
volatile struct SPI_REGS SpiaRegs;

#pragma DATA_SECTION("SysCtrlRegsFile");
volatile struct SYS_CTRL_REGS SysCtrlRegs;

#pragma DATA_SECTION("DevEmuRegsFile");
volatile struct DEV_EMU_REGS DevEmuRegs;

#pragma DATA_SECTION("CsmRegsFile");
volatile struct CSM_REGS CsmRegs;

#pragma DATA_SECTION("CsmPwlFile");
volatile struct CSM_PWL CsmPwl;

#pragma DATA_SECTION("FlashRegsFile");
volatile struct FLASH_REGS FlashRegs;

#pragma DATA_SECTION("XintfRegsFile");
volatile struct XINTF_REGS XintfRegs;

#pragma DATA_SECTION("XIntruptRegsFile");
volatile struct XINTRUPT_REGS XIntruptRegs;

#else	// "C"

#pragma DATA_SECTION(AdcRegs,"AdcRegsFile");
volatile struct ADC_REGS AdcRegs;

#pragma DATA_SECTION(CpuTimer0Regs,"CpuTimer0RegsFile");
volatile struct CPUTIMER_REGS CpuTimer0Regs;

#pragma DATA_SECTION(CpuTimer1Regs,"CpuTimer1RegsFile");
volatile struct CPUTIMER_REGS CpuTimer1Regs;

#pragma DATA_SECTION(CpuTimer2Regs,"CpuTimer2RegsFile");
volatile struct CPUTIMER_REGS CpuTimer2Regs;

#pragma DATA_SECTION(ECanaRegs,"ECanaRegsFile");
volatile struct ECAN_REGS ECanaRegs;

#pragma DATA_SECTION(ECanaLAMRegs,"ECanaRegsFile");
volatile struct LAM_REGS ECanaLAMRegs;

#pragma DATA_SECTION(ECanaMOTSRegs,"ECanaRegsFile");
volatile struct MOTS_REGS ECanaMOTSRegs;

#pragma DATA_SECTION(ECanaMOTORegs,"ECanaRegsFile");
volatile struct MOTO_REGS ECanaMOTORegs;

//#pragma DATA_SECTION(ECanaMboxes,"ECanaMboxesFile");		// PLC code stealing this chunk	
//volatile struct ECAN_MBOXES ECanaMboxes;						

#pragma DATA_SECTION(EvaRegs,"EvaRegsFile");
volatile struct EVA_REGS EvaRegs;

#pragma DATA_SECTION(EvbRegs,"EvbRegsFile");
volatile struct EVB_REGS EvbRegs;

#pragma DATA_SECTION(GpioDataRegs,"GpioDataRegsFile");
volatile struct GPIO_DATA_REGS GpioDataRegs;

#pragma DATA_SECTION(GpioMuxRegs,"GpioMuxRegsFile");
volatile struct GPIO_MUX_REGS GpioMuxRegs;

#pragma DATA_SECTION(McbspRegs,"McbspRegsFile");
volatile struct MCBSP_REGS McbspRegs;

#pragma DATA_SECTION(PieCtrlRegs,"PieCtrlRegsFile");
volatile struct PIE_CTRL_REGS PieCtrlRegs;

#pragma DATA_SECTION(PieVectTable,"PieVectTable");
struct PIE_VECT_TABLE PieVectTable;

#pragma DATA_SECTION(SciaRegs,"SciaRegsFile");
volatile struct SCI_REGS SciaRegs;

#pragma DATA_SECTION(ScibRegs,"ScibRegsFile");
volatile struct SCI_REGS ScibRegs;

#pragma DATA_SECTION(SpiaRegs,"SpiaRegsFile");
volatile struct SPI_REGS SpiaRegs;

#pragma DATA_SECTION(SysCtrlRegs,"SysCtrlRegsFile");
volatile struct SYS_CTRL_REGS SysCtrlRegs;

#pragma DATA_SECTION(DevEmuRegs,"DevEmuRegsFile");
volatile struct DEV_EMU_REGS DevEmuRegs;

#pragma DATA_SECTION(CsmRegs,"CsmRegsFile");
volatile struct CSM_REGS CsmRegs;

#pragma DATA_SECTION(CsmPwl,"CsmPwlFile");
volatile struct CSM_PWL CsmPwl;

#pragma DATA_SECTION(FlashRegs,"FlashRegsFile");
volatile struct FLASH_REGS FlashRegs;

#pragma DATA_SECTION(XintfRegs,"XintfRegsFile");
volatile struct XINTF_REGS XintfRegs;

#pragma DATA_SECTION(XIntruptRegs,"XIntruptRegsFile");
volatile struct XINTRUPT_REGS XIntruptRegs;

#endif

