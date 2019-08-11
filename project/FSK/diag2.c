/*-----------------------------------------
  Filename:		diag.c

  Description:	functions to save off diagnostic variable data.

  Copyright (C) 2000-2005 Texas Instruments Incorporated
  Texas Instruments Proprietary Information
  Use subject to terms and conditions of TI Software License Agreement
 
  Revision History:
  28Feb05	Hagen	Put #if around SaveTrace() 

-----------------------------------------*/
#ifdef MEX_COMPILE
	#include "psk_modem.h"
#else
	#include "main.h"
#endif


/*==========================================================================================
  Function:		SaveTrace()

  Description: 	This function stores data to the trace buffer 

  Revision History:
  06/07/04	HEM		New Function.  
==========================================================================================*/
#if TRACE_BUF_LEN > 0
	#ifdef DSP_COMPILE
		extern u16	uTraceIndex;		// <== declared in plc.c
		//	q16	qTraceMax[4] = {-32767, -32767, -32767, -32767};	//!!!DEBUG
		//	q16	qTraceMin[4] = {+32767, +32767, +32767, +32767};	//!!!DEBUG
	
		void SaveTraceF(u16 uData)
		{
			// u16	j = uTraceIndex & 3;						//!!!DEBUG
			// uTraceMax[j] = Max(     uData, uTraceMax[j]);	//!!!DEBUG Find largest unsigned value
			// qTraceMax[j] = Max((q16)uData, qTraceMax[j]);	//!!!DEBUG Find largest signed value
			// qTraceMin[j] = Min((q16)uData, qTraceMin[j]);	//!!!DEBUG Find smallest signed value
			
			upTraceBuffer[uTraceIndex++] = uData;
		
			if (uTraceIndex >= (((u16)(TRACE_BUF_LEN/4))*4))
				uTraceIndex = 0;
		}
	#endif
#else
	#define SaveTraceF( uData )			// dummy macro
#endif

// for MEX_COMPILE SaveTrace() is a macro defined in psk_macros.h


/*==========================================================================================
  Function:		SaveTrace()

  Description: 	This function stores data to the trace buffer 

  Revision History:
  06/07/04	HEM		New Function.  
==========================================================================================*/
#ifdef MEX_COMPILE
	void SaveTraceInit(u16 parm)
	{
		u16		n;
	
		diag.len = parm;
		diagArray = mxCreateDoubleMatrix(length, 1, mxREAL);
		diag.r = mxGetPr(diagArray);
		diag.beg = diag.r;
		for( n=0; n<diag.len; n++ )
			*(diag.r++) = 0;
		diag.r = diag.beg;
	}
#endif

#ifdef DSP_COMPILE
	#if TRACE_BUF_LEN > 0
		#define SaveTraceInit(parm)  uTraceIndex = parm;
	#else	
		#define SaveTraceInit(parm)  
	#endif
#endif



#ifdef MEX_COMPILE
void SaveTraceWorkspace(char *matName)
{
	mxSetName(diagArray, matName);
	mexPutArray(diagArray, "caller");
}

#else
	#define SaveTraceWorkspece(x)	(NOP)
#endif



#ifdef MEX_COMPILE
/*====================================================================
	Send diagnostic data to the matlab workspace

=====================================================================*/
void SaveVector( void *array, u16 len, char *matName, diagType flag)
{
	mxArray		*diagArray;
	dCplxPtr	diag;

	u16			n;
	u16			*unum;
	i16			*inum;
	iCplx		*jnum;

	switch( flag )
	{
	case diagU16:
		diagArray = mxCreateDoubleMatrix(len, 1, mxREAL);
		diag.r = mxGetPr(diagArray);
		unum = (u16 *)array;
		for( n = 0 ; n < len; n++ )
		{
			*diag.r++ = (double)(*unum++);
		}
		break;
	case diagI16:
		diagArray = mxCreateDoubleMatrix(len, 1, mxREAL);
		diag.r = mxGetPr(diagArray);
		inum = (i16 *)array;
		for( n = 0 ; n < len; n++ )
		{
			*diag.r++ = (double)(*inum++);
		}
		break;
	case diagICPLX:
		diagArray = mxCreateDoubleMatrix(len, 1, mxCOMPLEX);
		diag.r = mxGetPr(diagArray);
		diag.i = mxGetPi(diagArray);
		jnum = (iCplx *)array;
		for( n = 0 ; n < len; n++ )
		{
			*diag.r++ = (double)(jnum->re);
			*diag.i++ = (double)(jnum->im);
			jnum++;
		}
		break;
	default:
		postError("Unrecognized diag type.");
	}

	mxSetName(diagArray, matName);
	mexPutArray(diagArray, "caller");

}
#endif


#ifdef MEX_COMPILE
/*====================================================================
	send syntax error message back to Matlab
=====================================================================*/
void syntaxErrorMsg( void )
{
	char			prtStr[80];

	strcpy( prtStr, "Unrecognized command string.  Valid syntax:\n" );
	strcat( prtStr, "		userData	= mex_modem('rec',  recSignal, parmStruct)\n" ); 
	strcat( prtStr, "		txSignal	= mex_modem('xmit', userData,  parmStruct)\n" );
	strcat( prtStr, "		parmStruct	= mex_modem('parm')\n" );
	mexErrMsgTxt(prtStr);
	return;
}
#endif



