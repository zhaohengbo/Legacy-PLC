/*
//
//      TMDX BETA RELEASE
//      Intended for product evaluation purposes
//
//###########################################################################
//
// FILE:	EzDSP_RAM_lnk.cmd
//
// TITLE:	Linker Command File For F2810 eZdsp programs that run out of RAM
//          This linker file assumes the user is booting up in Jump to H0 mode
//
//###########################################################################
//
//  Ver | dd mmm yyyy | Who  | Description of changes
// =====|=============|======|===============================================
//  0.51| 02 Apr 2002 | L.H. | Original Release.
//  0.56| 20 May 2002 | L.H. | No change
//      | 12 Jul 2002 | E.O. | Doubled prog&data sections. Data in low
//      |             |      |   memory for easier use with tester.
//      | 18 Jul 2002 | E.O. | Created sections for trace buffer, and build info
//      | 07 Aug 2002 | M.H. | Rename McbspaRegsFile -> McbspRegsFile
//      | 06 Feb 2003 | M.H. | Moved .reset section after .text so processor     
//		|             |      |  reset restarts properly.
//      | 07 Oct 2004 | M.H. | Added FLASH sections, but don't use them. 
// -----|-------------|------|-----------------------------------------------
//###########################################################################
*/

MEMORY
{
PAGE 0 : 			/* ----- PROGRAM ----- */
   	OTP         	: origin = 0x3D7800, length = 0x000400     /*  1k  on-chip OTP   */
   	FLASHE      	: origin = 0x3E8000, length = 0x004000     /*  16k on-chip FLASH */
   	FLASHD      	: origin = 0x3EC000, length = 0x004000     /*  16k on-chip FLASH */
   	FLASHC      	: origin = 0x3F0000, length = 0x004000     /*  16k on-chip FLASH */
   	FLASHB      	: origin = 0x3F4000, length = 0x002000     /*  8k  on-chip FLASH */

   	FLASHRESET  	: origin = 0x3F7FF6, length = 0x000002     /* Part of FLASHA.  Used for "boot to Flash" bootloader mode. */
	   FLASHPW			: origin = 0x3F7FF8, length = 0x000008	    /* Security Password (DON'T TOUCH!) */
   	
   	PRAMH0       	: origin = 0x3F8000, length = 0x002000     /* 8k on-chip SARAM block H0 */
   	
   	ROM         	: origin = 0x3FF000, length = 0x000FC0     /* <4k  Boot ROM available */
   	RESET       	: origin = 0x3FFFC0, length = 0x000002     /*      part of boot ROM ) */
   	VECTORS     	: origin = 0x3FFFC2, length = 0x00003E     /*      part of boot ROM ) */

PAGE 1 : 			/* ----- DATA ----- */
	/* SARAM */
	RAMM0A			: origin = 0x000000, length = 0x000020			/*  1k on-chip RAM M0 */
	RAMM0B			: origin = 0x000020, length = 0x0003E0
	RAMM1				: origin = 0x000400, length = 0x000400			/*  1k on-chip RAM M1 */
   FLASHA      	: origin = 0x3F6000, length = 0x001FF6     	/* <8k  on-chip FLASH */

	/* Peripheral Frame 0: */
	DEV_EMU    		: origin = 0x000880, length = 0x000180
	FLASH_REGS 		: origin = 0x000A80, length = 0x000060
	CSM        		: origin = 0x000AE0, length = 0x000010
	XINTF      		: origin = 0x000B20, length = 0x000020
	CPU_TIMER0 		: origin = 0x000C00, length = 0x000008
	CPU_TIMER1 		: origin = 0x000C08, length = 0x000008		 
	CPU_TIMER2 		: origin = 0x000C10, length = 0x000008		 
	PIE_CTRL   		: origin = 0x000CE0, length = 0x000020
	PIE_VECT   		: origin = 0x000D00, length = 0x000100

	/* Peripheral Frame 1: */
	ECAN_A     		: origin = 0x006000, length = 0x000100
	ECAN_AMBOX 		: origin = 0x006100, length = 0x000100

	/* Peripheral Frame 2: */
	SYSTEM     		: origin = 0x007010, length = 0x000020
	SPI_A      		: origin = 0x007040, length = 0x000010
	SCI_A      		: origin = 0x007050, length = 0x000010
	XINTRUPT   		: origin = 0x007070, length = 0x000010
	GPIOMUX    		: origin = 0x0070C0, length = 0x000020
	GPIODAT    		: origin = 0x0070E0, length = 0x000020
	ADC        		: origin = 0x007100, length = 0x000020
	EV_A       		: origin = 0x007400, length = 0x000040
	EV_B       		: origin = 0x007500, length = 0x000040
	SPI_B      		: origin = 0x007740, length = 0x000010
	SCI_B      		: origin = 0x007750, length = 0x000010
	MCBSP_A    		: origin = 0x007800, length = 0x000040

	/* CSM Password Locations */
	CSM_PWL    		: origin = 0x3F7FF8, length = 0x000008

	/* SARAM */     
	/*DRAML0A    		: origin = 0x8000, length = 0x000040   */
	/*DRAML0     		: origin = 0x8040, length = 0x0009C0	*/
	/*DRAML1     		: origin = 0x9000, length = 0x001000	*/

	DRAML0    		: origin = 0x8000, length = 0x000400   
	DRAML1     		: origin = 0x8400, length = 0x001C00	
}
 
 
SECTIONS
{
   /* Allocate program areas: */
   .reset           : > PRAMH0,      	PAGE = 0
   .text            : > PRAMH0,      	PAGE = 0
   .cinit           : > PRAMH0,      	PAGE = 0
   .switch          : > PRAMH0,      	PAGE = 0
   ramfuncs         : > PRAMH0, 			PAGE = 0

   /* Allocate data areas: */
   .build_info		  : > RAMM0A,		 	PAGE = 1
   .fir_out_old	  : > RAMM0B,			PAGE = 1
   .const           : > RAMM0B,      	PAGE = 1	/* Put this is different section than fir_databuff to speed up FIR*/
   .stack           : > RAMM1,       	PAGE = 1
 /*.fir_databuff    : > DRAML0A,		PAGE = 1 */	/* Make this first item in section.  MUST be multiple of 0x0100 */
   .bss             : > DRAML0,      	PAGE = 1
   .ebss            : > DRAML0,      	PAGE = 1
   .econst          : > DRAML0,      	PAGE = 1      
   .sysmem          : > DRAML0,      	PAGE = 1
   .trc_buff		  : > DRAML1,			PAGE = 1
   
   /* Allocate Peripheral Frame 0 Register Structures:   */
   DevEmuRegsFile    : > DEV_EMU,    	PAGE = 1
   FlashRegsFile     : > FLASH_REGS, 	PAGE = 1
   CsmRegsFile       : > CSM,        	PAGE = 1
   XintfRegsFile     : > XINTF,      	PAGE = 1
   CpuTimer0RegsFile : > CPU_TIMER0, 	PAGE = 1      
   CpuTimer1RegsFile : > CPU_TIMER1, 	PAGE = 1      
   CpuTimer2RegsFile : > CPU_TIMER2, 	PAGE = 1      
   PieCtrlRegsFile   : > PIE_CTRL,   	PAGE = 1      
   PieVectTable      : > PIE_VECT,   	PAGE = 1

   /* Allocate Peripheral Frame 2 Register Structures:   */
   ECanaRegsFile     : > ECAN_A,      	PAGE = 1   
   ECanaMboxesFile   : > ECAN_AMBOX   	PAGE = 1

   /* Allocate Peripheral Frame 1 Register Structures:   */
   SysCtrlRegsFile   : > SYSTEM,     	PAGE = 1
   SpiaRegsFile      : > SPI_A,      	PAGE = 1
   SciaRegsFile      : > SCI_A,      	PAGE = 1
   XIntruptRegsFile  : > XINTRUPT,   	PAGE = 1
   GpioMuxRegsFile   : > GPIOMUX,    	PAGE = 1
   GpioDataRegsFile  : > GPIODAT     	PAGE = 1
   AdcRegsFile       : > ADC,        	PAGE = 1
   EvaRegsFile       : > EV_A,       	PAGE = 1
   EvbRegsFile       : > EV_B,       	PAGE = 1
   ScibRegsFile      : > SCI_B,      	PAGE = 1
   McbspRegsFile     : > MCBSP_A,    	PAGE = 1

   /* CSM Password Locations */
   CsmPwlFile      : > CSM_PWL,     	PAGE = 1
}

