//==========================================================================================
// Filename:		gpio.c for TMS320F2808
//
// Description:		Functions used to configure the gpio ports.
//
// Copyright (C) 2002 Texas Instruments Incorporated
// Texas Instruments Proprietary Information
// Use subject to terms and conditions of TI Software License Agreement
//
// Revision History:
// 06/20/02 EGO		Started file.
// 11/1x/04	HEM		Removed unused functions left over from CAN project.
// 07/22/05 AM		Converted to TMS320F2808
//==========================================================================================

#include "main.h"


//==========================================================================================
// Local function prototypes
//==========================================================================================


//==========================================================================================
// External Variables
//==========================================================================================


//==========================================================================================
// Local constants
//==========================================================================================
// Each mux register can be set to either its peripheral output, or to a GPIO.
#define MUX_GPIO		(0)
#define MUX_PERIPHERAL	(1)

// The direction register is used to set a GPIO as either an input or an output.
#define GPIO_INPUT		(0)
#define GPIO_OUTPUT 	(1)


//==========================================================================================
// Function:		InitGpio() for TMS320F2808
//
// Description: 	Initializes the GPIO registers.  The function determines which
//					IO pins should be used for their predefined peripheral output, and which
//					are to be GPIO.  For GPIO further configuration is required as described
//					below.
//
//					Should be called once at start-up, before main loop starts.
// *** END WARNING - TUTORIAL OFF
//
//
// Revision History:
// 06/20/02	EGO		New Function.
// 06/20/02 HEM		Added ADC_CALIB bit on G4 output and DRV_WAKEUP on G5 output.
// 06/21/02 HEM/EGO	Explicitly declare all used pins and default all unused pins to GP inputs. 
// 06/24/02 HEM		Swapped error LEDs with laser disable pins.
// 07/09/02 EGO		Deleted dead code.
// 07/31/02 HEM		Moved LSR0_DIS and LSR2_DIS to match new board.
//					Added DRV_SHUTDOWN0 thru 3.
// 					Configured MCLKXA and MFSXA pins as peripherals instead of GPIO.
// 08/05/02 HEM		Added DAC_EN to control setpoint DAC enable.
// 08/06/02 HEM		Configured MDXA pin as peripheral instead of GPIO.
// 10/17/02 HEM		Moved UART RTS & CTS to free up CANTX & CANRX.
//					Eliminated laser disable pins.
// 10/21/02 HEM		Added CANRX_LED, CANTX_LED, STATUS_LED0 thru STATUS_LED2.
// 11/04/02 HEM		Changed GPIO for fan control switches and indicators.
// 12/20/02 HEM		Enable CAN termination resistor.
// 01/09/03 HEM		Defined GPIO to control CAN Bus Corrupter.
// 03/05/03 KKN		Changed the initial state of the CAN_TERM_EN GPIO to 1.
//					This will allow the users to enable the CAN terminator from the DIP switch.
// 03/20/03	HEM		Stripped out unused stuff from TEC project.
// 07/22/05 AM		Modified for 280x operation.
//==========================================================================================
void InitGpio(void)
{
	EALLOW;
	{
		// To prevent any possible driver conflicts, set all gpio pins to input at powerup,
		// then switch appropriate pins to either peripheral or general purpose outputs.
		
		// Set GPIO as inputs using GPACTRL for GPIO00 to GIPO31
		GpioCtrlRegs.GPAMUX1.all = 0;
		
		// Set GPIO as inputs using GPBCTRL for GPIO32 to GPIO34
		GpioCtrlRegs.GPBMUX1.all = 0;

		// Set GPIO as inputs.
//		GpioMuxRegs.GPADIR.all = 0;
//		GpioMuxRegs.GPBDIR.all = 0;
//		GpioMuxRegs.GPDDIR.all = 0;
//		GpioMuxRegs.GPEDIR.all = 0;
//		GpioMuxRegs.GPFDIR.all = 0;
//		GpioMuxRegs.GPGDIR.all = 0;

		// Initialize all to GPIO.
//		GpioMuxRegs.GPAMUX.all = 0;
//		GpioMuxRegs.GPBMUX.all = 0;
//		GpioMuxRegs.GPDMUX.all = 0;
//		GpioMuxRegs.GPEMUX.all = 0;
//		GpioMuxRegs.GPFMUX.all = 0;
//		GpioMuxRegs.GPGMUX.all = 0;


		// Switch desired signals to GPIO as needed.
		// Should make an #define entry in main.h for each GPIO as well.  
		// For example see the definition of "UART_RTS".

		// Configure GPIO A bits 0-5 as peripherals for outputs PWM1-6.
		
		GpioCtrlRegs.GPAMUX1.bit.GPIO0 = MUX_PERIPHERAL;
		GpioCtrlRegs.GPAMUX1.bit.GPIO1 = MUX_PERIPHERAL;
		GpioCtrlRegs.GPAMUX1.bit.GPIO2 = MUX_PERIPHERAL;
		GpioCtrlRegs.GPAMUX1.bit.GPIO3 = MUX_PERIPHERAL;
		GpioCtrlRegs.GPAMUX1.bit.GPIO4 = MUX_PERIPHERAL;
		GpioCtrlRegs.GPAMUX1.bit.GPIO5 = MUX_PERIPHERAL;
		
//		GpioMuxRegs.GPAMUX.bit.PWM1_GPIOA0 = MUX_PERIPHERAL;
//		GpioMuxRegs.GPAMUX.bit.PWM2_GPIOA1 = MUX_PERIPHERAL;
//		GpioMuxRegs.GPAMUX.bit.PWM3_GPIOA2 = MUX_PERIPHERAL;
//		GpioMuxRegs.GPAMUX.bit.PWM4_GPIOA3 = MUX_PERIPHERAL;
//		GpioMuxRegs.GPAMUX.bit.PWM5_GPIOA4 = MUX_PERIPHERAL;
//		GpioMuxRegs.GPAMUX.bit.PWM6_GPIOA5 = MUX_PERIPHERAL;

		// Configure GPIO A bits 6&7 as peripherals for T1PWM and T2PWM outputs
//		GpioMuxRegs.GPAMUX.bit.T1PWM_GPIOA6 = MUX_PERIPHERAL;
//		GpioMuxRegs.GPAMUX.bit.T2PWM_GPIOA7 = MUX_PERIPHERAL;


		// Configure GPIO A bits 8-10 as outputs for PLC status LEDs
		
		GpioCtrlRegs.GPAMUX2.bit.GPIO20 = MUX_GPIO;		
		GpioCtrlRegs.GPADIR.bit.GPIO20 = GPIO_OUTPUT;
		
//		GpioMuxRegs.GPAMUX.bit.CAP1Q1_GPIOA8 = MUX_GPIO;
//		GpioMuxRegs.GPADIR.bit.GPIOA8 = GPIO_OUTPUT;

		PLC_RX_GOOD_LED = 0;		// Set initial value.


		GpioCtrlRegs.GPAMUX2.bit.GPIO21 = MUX_GPIO;
		GpioCtrlRegs.GPADIR.bit.GPIO21 = GPIO_OUTPUT;

//		GpioMuxRegs.GPAMUX.bit.CAP2Q2_GPIOA9 = MUX_GPIO;
//		GpioMuxRegs.GPADIR.bit.GPIOA9 = GPIO_OUTPUT;

		PLC_RX_BUSY_LED= 0;		// Set initial value.


		GpioCtrlRegs.GPAMUX2.bit.GPIO23 = MUX_GPIO;
		GpioCtrlRegs.GPADIR.bit.GPIO23 = GPIO_OUTPUT;
		
//		GpioMuxRegs.GPAMUX.bit.CAP3QI1_GPIOA10 = MUX_GPIO;
//		GpioMuxRegs.GPADIR.bit.GPIOA10 = GPIO_OUTPUT;

		PLC_TX_LED = 0;		// Set initial value.


		// Configure GPIO13 bit 11 as output for PLC line driver enable
		
		GpioCtrlRegs.GPAMUX1.bit.GPIO13 = MUX_GPIO;
//		GpioCtrlRegs.GPAMUX1.bit.GPIO13 = GPIO_OUTPUT;
		GpioCtrlRegs.GPADIR.bit.GPIO13 = GPIO_OUTPUT;
		
//		GpioMuxRegs.GPAMUX.bit.TDIRA_GPIOA11 = MUX_GPIO;
//		GpioMuxRegs.GPADIR.bit.GPIOA11 = GPIO_OUTPUT;

		TX_BIAS_EN = 0;	// Set initial value.


		// Configure GPIO A bit 12-15 as outputs for status LEDs'
//		GpioMuxRegs.GPAMUX.bit.TCLKINA_GPIOA12 = MUX_GPIO;
//		GpioMuxRegs.GPADIR.bit.GPIOA12 = GPIO_OUTPUT;
//  		LAMP_ON_LED = 0;		// Set initial value.

//		GpioMuxRegs.GPAMUX.bit.C1TRIP_GPIOA13 = MUX_GPIO;
//		GpioMuxRegs.GPADIR.bit.GPIOA13 = GPIO_OUTPUT;
  //		LAMP_MODE_LED = 0;		// Set initial value.

	//	GpioMuxRegs.GPAMUX.bit.C2TRIP_GPIOA14 = MUX_GPIO;
	//	GpioMuxRegs.GPADIR.bit.GPIOA14 = GPIO_OUTPUT;
  	//	FAN_ON_LED = 0;		// Set initial value.

	//	GpioMuxRegs.GPAMUX.bit.C3TRIP_GPIOA15 = MUX_GPIO;
	//	GpioMuxRegs.GPADIR.bit.GPIOA15 = GPIO_OUTPUT;
  	//	FAN_MODE_LED = 0;		// Set initial value.
  		
 
// Configure GPIO10 as PWMEPWM6A for Lamp Control
 		GpioCtrlRegs.GPAMUX1.bit.GPIO10 = MUX_PERIPHERAL;
 		
 		
// 		Configure GPIO B bits 0-3 as peripherals for outputs PWM7-10. 		
//		GpioMuxRegs.GPBMUX.bit.PWM7_GPIOB0  = MUX_PERIPHERAL;
//		GpioMuxRegs.GPBMUX.bit.PWM8_GPIOB1  = MUX_PERIPHERAL;
//		GpioMuxRegs.GPBMUX.bit.PWM9_GPIOB2  = MUX_PERIPHERAL;
//		GpioMuxRegs.GPBMUX.bit.PWM10_GPIOB3 = MUX_PERIPHERAL;

		// Configure GPIO B bits 4 - 7 as outputs to control CAN bus corrupter
//		GpioMuxRegs.GPBMUX.bit.PWM11_GPIOB4 = MUX_GPIO;
//		GpioMuxRegs.GPBDIR.bit.GPIOB4 = GPIO_OUTPUT;
//		CORRUPTER_MODE0 = 0;		// Set initial value.
 
//		GpioMuxRegs.GPBMUX.bit.PWM12_GPIOB5 = MUX_GPIO;
//		GpioMuxRegs.GPBDIR.bit.GPIOB5 = GPIO_OUTPUT;
//		CORRUPTER_MODE1 = 0;		// Set initial value.

//		GpioMuxRegs.GPBMUX.bit.T3PWM_GPIOB6 = MUX_GPIO;
//		GpioMuxRegs.GPBDIR.bit.GPIOB6 = GPIO_OUTPUT;
//		CORRUPTER_MODE2 = 0;		// Set initial value.

//		GpioMuxRegs.GPBMUX.bit.T4PWM_GPIOB7 = MUX_GPIO;
//		GpioMuxRegs.GPBDIR.bit.GPIOB7 = GPIO_OUTPUT;
//		CORRUPTER_EN = 0;		// Set initial value.



// Configure GPIO B bits 8-15 as inputs for address, configuration, and switches
		
		GpioCtrlRegs.GPAMUX2.bit.GPIO27 = MUX_GPIO;   // JP200 1-2
		GpioCtrlRegs.GPADIR.bit.GPIO27 	= GPIO_INPUT;  
// 		INPUT_B8	GpioDataRegs.GPADAT.bit.GPIO27
		
		GpioCtrlRegs.GPAMUX1.bit.GPIO6 	= MUX_GPIO;		//JP200 3-4
		GpioCtrlRegs.GPADIR.bit.GPIO6 	= GPIO_INPUT;
//		INPUT_B9		GpioDataRegs.GPADAT.bit.GPIO6
		
		GpioCtrlRegs.GPAMUX1.bit.GPIO15 = MUX_GPIO;		// JP200 5-6
		GpioCtrlRegs.GPADIR.bit.GPIO15 	= GPIO_INPUT;	
//		INPUT_B10		GpioDataRegs.GPADAT.bit.GPIO15
		
		GpioCtrlRegs.GPAMUX1.bit.GPIO7 	= 	MUX_GPIO;   // JP200 7-8
		GpioCtrlRegs.GPADIR.bit.GPIO7 	= 	GPIO_INPUT;
//		INPUT_B11		GpioDataRegs.GPADAT.bit.GPIO7
		
//		GpioMuxRegs.GPBMUX.bit.CAP4Q1_GPIOB8 = MUX_GPIO;
//		GpioMuxRegs.GPBDIR.bit.GPIOB8 = GPIO_INPUT;

//		GpioMuxRegs.GPBMUX.bit.CAP5Q2_GPIOB9 = MUX_GPIO;
//		GpioMuxRegs.GPBDIR.bit.GPIOB9 = GPIO_INPUT;

//		GpioMuxRegs.GPBMUX.bit.CAP6QI2_GPIOB10 = MUX_GPIO;
//		GpioMuxRegs.GPBDIR.bit.GPIOB10 = GPIO_INPUT;

//		GpioMuxRegs.GPBMUX.bit.TDIRB_GPIOB11 = MUX_GPIO;
//		GpioMuxRegs.GPBDIR.bit.GPIOB11 = GPIO_INPUT;

//		GpioMuxRegs.GPBMUX.bit.TCLKINB_GPIOB12 = MUX_GPIO;
//		GpioMuxRegs.GPBDIR.bit.GPIOB12 = GPIO_INPUT;

//		GpioMuxRegs.GPBMUX.bit.C4TRIP_GPIOB13 = MUX_GPIO;
//		GpioMuxRegs.GPBDIR.bit.GPIOB13 = GPIO_INPUT;

//		GpioMuxRegs.GPBMUX.bit.C5TRIP_GPIOB14 = MUX_GPIO;
//		GpioMuxRegs.GPBDIR.bit.GPIOB14 = GPIO_INPUT;

//		GpioMuxRegs.GPBMUX.bit.C6TRIP_GPIOB15 = MUX_GPIO;
//		GpioMuxRegs.GPBDIR.bit.GPIOB15 = GPIO_INPUT;


		// D0  T1CTRIP_PDPA_GPIOD0	 Available but unassigned
		// D1  T2CTRIP_SOCA_GPIOD1	 Available but unassigned
		//GpioMuxRegs.GPDMUX.bit.T2CTRIP_SOCA_GPIOD1 = MUX_PERIPHERAL;	//!!! DEBUG Spit out SOC signal to GPIO pin
		// D5  T3CTRIP_PDPB_GPIOD5	 Available but unassigned
		// D6  T4CTRIP_SOCB_GPIOD6	 Available but unassigned


		// Configure GPIO E bit 0 as peripheral for input XINT1.
//		GpioMuxRegs.GPEMUX.bit.XINT1_XBIO_GPIOE0 = MUX_PERIPHERAL;

		// E1 XINT2_ADCSOC_GPIOE1 Available but unassigned
		//GpioMuxRegs.GPEMUX.bit.XINT2_ADCSOC_GPIOE1 = MUX_PERIPHERAL;	//!!! DEBUG Spit out SOC signal to GPIO pin
		// E2 XNMI_XINT13_GPIOE2  Available but unassigned

		// F0 SPISIMOA_GPIOF0	  Available but unassigned	
		// F1 SPISOMIA_GPIOF1	  Available but unassigned	

 		// Configure GPIO16 as output 'UART_RTS'
 		
 		GpioCtrlRegs.GPAMUX2.bit.GPIO16 = MUX_GPIO;
 		GpioCtrlRegs.GPADIR.bit.GPIO16 = GPIO_OUTPUT;
 		
//		GpioMuxRegs.GPFMUX.bit.SPISIMOA_GPIOF0 = MUX_GPIO;
//		GpioMuxRegs.GPFDIR.bit.GPIOF0 = GPIO_OUTPUT;
//		GpioDataRegs.GPADAT.bit.GPIO16

		UART_RTS = 0;		// Set initial value.

		// Configure GPIO17 as input 'UART_CTS'
		GpioCtrlRegs.GPAMUX2.bit.GPIO17 = MUX_GPIO;
		GpioCtrlRegs.GPADIR.bit.GPIO17 = GPIO_INPUT;
		
//		GpioMuxRegs.GPFMUX.bit.SPISOMIA_GPIOF1 = MUX_GPIO;
//		GpioMuxRegs.GPFDIR.bit.GPIOF1 = GPIO_INPUT;

		// Configure GPIO F bit 2 as output 'CANTX_LED'
//		GpioMuxRegs.GPFMUX.bit.SPICLKA_GPIOF2 = MUX_GPIO;
//		GpioMuxRegs.GPFDIR.bit.GPIOF2 = GPIO_OUTPUT;
//		CANTX_LED = 0;		// Set initial value.

		// Configure GPIO F bit 3 as output 'CANRX_LED'
//		GpioMuxRegs.GPFMUX.bit.SPISTEA_GPIOF3 = MUX_GPIO;
//		GpioMuxRegs.GPFDIR.bit.GPIOF3 = GPIO_OUTPUT;
//		CANRX_LED = 0;		// Set initial value.


// Configure GPIO29 SCITXDA & SCIRXDA
		GpioCtrlRegs.GPAMUX2.bit.GPIO29 = MUX_PERIPHERAL; // SCITXDA
		GpioCtrlRegs.GPAMUX2.bit.GPIO28 = MUX_PERIPHERAL; // SCIRXDA

 		// Configure GPIO F bit 4 as peripheral for output SCITXDA.
//		GpioMuxRegs.GPFMUX.bit.SCITXDA_GPIOF4 = MUX_PERIPHERAL;

 		// Configure GPIO F bit 5 as peripheral for input SCIRXDA.
//		GpioMuxRegs.GPFMUX.bit.SCIRXDA_GPIOF5 = MUX_PERIPHERAL;

 		// Configure GPIO F bit 6 as peripheral for output CANTXDA.
//		GpioMuxRegs.GPFMUX.bit.CANTXA_GPIOF6 = MUX_PERIPHERAL;

 		// Configure GPIO F bit 7 as peripheral for input CANRXDA.
//		GpioMuxRegs.GPFMUX.bit.CANRXA_GPIOF7 = MUX_PERIPHERAL;

 		// Configure GPIO F bit 8 as peripheral for output MCLKXA.
//		GpioMuxRegs.GPFMUX.bit.MCLKXA_GPIOF8 = MUX_PERIPHERAL;


		// F9 MCLKRA_GPIOF9 	Available but unassigned


 		// Configure GPIO F bit 10 as peripheral for output MFSXA.
//		GpioMuxRegs.GPFMUX.bit.MFSXA_GPIOF10 = MUX_PERIPHERAL;

		// F11 MFSRA_GPIOF11 	Available but unassigned


//		// Configure GPIO F bit 12 as peripheral for output MDXA.
//		GpioMuxRegs.GPFMUX.bit.MDXA_GPIOF12 = MUX_PERIPHERAL;


		// F13 MDRA_GPIOF13 	Available but unassigned


  		// Configure GPIO F bit 14 as peripheral for output XF.
//		GpioMuxRegs.GPFMUX.bit.XF_GPIOF14 = MUX_PERIPHERAL;


		// G4 SCITXDB_GPIOG4 	Available but unassigned
		// G5 SCIRXDB_GPIOG5 	Available but unassigned
	}

	EDIS;

	return;
}


