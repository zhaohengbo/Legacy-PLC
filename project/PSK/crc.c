//==========================================================================================
// Filename:		crc.c
//
// Description:		Functions for calculating the Cyclic Redundancy Check in a 
//					single-carrier-frequency power line modem.
//
// Copyright (C) 2000 - 2004 Texas Instruments Incorporated
// Texas Instruments Proprietary Information
// Use subject to terms and conditions of TI Software License Agreement
//
// Revision History:
// 08/27/04	HEM		New file.
// 15Feb05	Hagen	allocated CRCtableArray in CAN mailbox memory space
//==========================================================================================

#include "main.h"
//#include <string.h>					// contains memset()


#pragma DATA_SECTION(CRCtableArray,"ECanaMboxesFile");		// PLC code stealing this chunk	from ECAN mailbox
u16	CRCtableArray[256];				// CRC table buffer

#define	BYTE_LEN			8
#define	CRC_LEN				16		// length in bits of CRC word
#define	CRC_POLYNOMIAL	0x1021		// generator polynomial	(0x1021 = x^16 + X^12 + X^5 + X^1)
#define	CRC_TOPBIT		0x8000		// MSB
#define	CRC_REG_INIT	0xFFFF		// initial value for control register
#define	CRC_BLOCK_LEN		16		// num bytes over which to calc a CRC word



//==========================================================================================
// Function:		InitCRCtable()
//
// Description: 	Init CRC-16 table
//
// Revision History:
// 08/27/04	HEM		New function, copied from PLC project.
//==========================================================================================
void InitCRCtable(void)
//void initCRCtable(u16 *CRCtable)
{
	u16			reg;		// working register
	u16			n, b;		// counters

	reg = 0;
    for( n = 0; n < 256; n++ )
	{
        reg = n << 8;
        for( b = 0; b < 8; b++ )
		{
            if( reg & CRC_TOPBIT )
				reg = (reg << 1) ^ CRC_POLYNOMIAL;
			else
				reg <<= 1;
        }
//        *CRCtable++ = reg;
        CRCtableArray[n] = reg;
    }    
	return;
}


//==========================================================================================
// Function:		CalcCRC()
//
// Description: 	Calculate CRC check bytes. Data come in as 8-bit byte.
//					CRC returned is 16 bits. 
//
// Revision History:
// 09/01/04	HEM		New function, copied from PLC project, then mostly re-written.
//==========================================================================================
u16 CalcCRC(u16 uPlcMode, u16 numBytes)
{
	u16	byteCount;
	u16	dataByte;
	u16	reg;
	u16	*pUserData;
	

/*
	u16	shift;


	if (uPlcMode == RX_MODE)
	{
		pUserData = rxDataArray;
		shift =	8;
	}
	else
	{
		pUserData = txUserDataArray;
		shift = 0;
	}


//	//---- init parity table, if required ----------------------
//	if( CRCtableArray[1] == 0 )
//		InitCRCtable();
	
	// --------------------------------------------------------------
	// Read through the data to generate the CRC word.
	// Append the CRC word at the end of the user data in the data buffer
	// --------------------------------------------------------------
	reg = CRC_REG_INIT;
	for( byteCount = 0; byteCount < (numBytes+1); byteCount++)
	{
		dataByte = (*pUserData++) >> shift;		// choose proper byte
		reg = (reg << BYTE_LEN) 
			^ CRCtableArray[ (reg >> (CRC_LEN-BYTE_LEN)) ] ^ dataByte;
	}
*/	

#define START_IX	0	//1
#define OVERHANG	0	//1
	reg = CRC_REG_INIT;
	if (uPlcMode == RX_MODE)
	{
		pUserData = rxUserDataArray+START_IX;	
		for( byteCount = START_IX; byteCount < (numBytes+OVERHANG); byteCount++)
		{
			dataByte = (*pUserData++) >> 8;		// choose high byte
			reg = (reg << BYTE_LEN) 
				^ CRCtableArray[ (reg >> (CRC_LEN-BYTE_LEN)) ] ^ dataByte;
		}
	}
	else
	{
		pUserData = txUserDataArray+START_IX;	
		for( byteCount = START_IX; byteCount < (numBytes+OVERHANG); byteCount++)
		{
			dataByte = (*pUserData++) >> 0;		// choose low byte
			reg = (reg << BYTE_LEN) 
				^ CRCtableArray[ (reg >> (CRC_LEN-BYTE_LEN)) ] ^ dataByte;
		}	
	}

	return(reg);
}


/*
//==========================================================================================
// Function:		AppendCRCBytes()
//
// Description: 	Calculate CRC check bytes and append to end of message.
//					Data come in as 16 bit words. Take each byte and calc parity bytes
//					Calc CRC on whole words.
//
// Revision History:
// 08/27/04	HEM		New function, copied from PLC project.
//==========================================================================================
void AppendCRCBytes(u16 *pUserData, u16 numWords)
{
	u16				wordCount = 0;
	u16				dataByte;
	u16				reg;
	u16				*parity;

	//---- init parity table ----------------------
	if( CRCtableArray[1] == 0 )
		InitCRCtable();
//		initCRCtable( CRCtableArray );
		
	//---- point to appended parity part of data buffer and zero out ---
	parity = pUserData + numWords;
//	*parity  = 0;
//	memset(parity, 0, (DATA_BUFFER_LEN-numWords)*sizeof(u16));	
	
	//--------------------------------------------------------------
	// Read through the data to generate the CRC word.
	// Append the CRC word at the end of the user data in the data buffer
	//--------------------------------------------------------------
	reg = CRC_REG_INIT;
	for( wordCount = 0; wordCount < (numWords+1); wordCount++ )
	{
	
		dataByte = (*pUserData) >> BYTE_LEN;// high data byte
		reg = (reg << BYTE_LEN) 
			^ CRCtableArray[ (reg >> (CRC_LEN-BYTE_LEN)) ] ^ dataByte;

		dataByte = (*pUserData++) & 0x00FF;	// low data byte
		reg = (reg << BYTE_LEN) 
			^ CRCtableArray[ (reg >> (CRC_LEN-BYTE_LEN)) ] ^ dataByte;

	}
	*parity = reg;	
	return;
}


//==========================================================================================
// Function:		CompareCRCBytes()   
//
// Description: 	Calculate CRC check bytes and compare to received CRC.
//
// Revision History:
// 08/27/04	HEM		New function, copied from PLC project.
//==========================================================================================
u16 CompareCRCBytes(u16 *pUserData, u16 numBytes)
{
	u16				byteCount = 0;
	u16				dataByte;
	u16				reg;

//	//---- init parity table ----------------------
//	if( CRCtableArray[1] == 0 )
//		InitCRCtable();
		
	//--------------------------------------------------------------
	// Read through the data to calculate the CRC word.  Compare it 
	// to the CRC appended at transmit.
	//--------------------------------------------------------------
	reg = CRC_REG_INIT;
	for( byteCount = 0; byteCount < (numBytes+1); byteCount++)
	{
		dataByte = (*pUserData) >> BYTE_LEN;// high data byte
		reg = (reg << BYTE_LEN) 
			^ CRCtableArray[ (reg >> (CRC_LEN-BYTE_LEN)) ] ^ dataByte;

		dataByte = (*pUserData++) & 0x00FF;	// low data byte
		reg = (reg << BYTE_LEN) 
			^ CRCtableArray[ (reg >> (CRC_LEN-BYTE_LEN)) ] ^ dataByte;
	}

	return reg;
}

*/





