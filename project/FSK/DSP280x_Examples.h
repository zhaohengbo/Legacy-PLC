// TI File $Revision: /main/2 $
// Checkin $Date: December 1, 2004   13:29:45 $
//###########################################################################
//
// FILE:   DSP280x_Examples.h
//
// TITLE:  DSP280x Device Definitions.
//
//###########################################################################
// $TI Release: DSP280x V1.00 $
// $Release Date: December 9, 2004 $
//###########################################################################

#ifndef DSP280X_EXAMPLES_H
#define DSP280X_EXAMPLES_H


#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------------
      Specify the clock rate of the CPU (SYSCLKOUT) in nS.

      Take into account the input clock frequency and the PLL multiplier
      selected in step 1.
 
      Use one of the values provided, or define your own.
      The trailing L is required tells the compiler to treat 
      the number as a 64-bit value.  

      Only one statement should be uncommented.

      Example:  CLKIN is a 10MHz crystal. 
 
                In step 1 the user specified PLLCR = 0xA for a 
                100Mhz CPU clock (SYSCLKOUT = 100MHz).  

                In this case, the CPU_RATE will be 10.000L
                Uncomment the line:  #define CPU_RATE   10.000L
-----------------------------------------------------------------------------*/

#define CPU_RATE   10.000L   // for a 100MHz CPU clock speed (SYSCLKOUT)
//#define CPU_RATE   13.330L   // for a 75MHz CPU clock speed (SYSCLKOUT)
//#define CPU_RATE   20.000L   // for a 50MHz CPU clock speed  (SYSCLKOUT)
//#define CPU_RATE   33.333L   // for a 30MHz CPU clock speed  (SYSCLKOUT)
//#define CPU_RATE   41.667L   // for a 24MHz CPU clock speed  (SYSCLKOUT)
//#define CPU_RATE   50.000L   // for a 20MHz CPU clock speed  (SYSCLKOUT)
//#define CPU_RATE   66.667L   // for a 15MHz CPU clock speed  (SYSCLKOUT)
//#define CPU_RATE  100.000L   // for a 10MHz CPU clock speed  (SYSCLKOUT)

//----------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Include Example Header Files:
//

#include "DSP280x_GlobalPrototypes.h"         // Prototypes for global functions within the 
                                              // .c files.

#include "DSP280x_ePwm_defines.h"             // Macros used for PWM examples. 
#include "DSP280x_I2C_defines.h"              // Macros used for I2C examples.

// Include files not used with DSP/BIOS
#ifndef DSP28_BIOS
#include "DSP280x_DefaultISR.h"
#endif


// DO NOT MODIFY THIS LINE.  
#define DELAY_US(A)  DSP28x_usDelay(((((long double) A * 1000.0L) / (long double)CPU_RATE) - 9.0L) / 5.0L)


#ifdef __cplusplus
}
#endif /* extern "C" */

#endif  // end of DSP280x_EXAMPLES_H definition


//===========================================================================
// End of file.
//===========================================================================
