/********************************************************************
* FileName:		main.c1
* Dependencies:    
* Processor:		dsPIC30F Family
* Hardware:		dsPICDEM 2
* Compiler:		C30 2.01
* Company:		Microchip Technology, Inc.
*
* Software License Agreement
*
* The software supplied herewith by Microchip Technology Incorporated
* (the “Company”) for its PICmicro® Microcontroller is intended and
* supplied to you, the Company’s customer, for use solely and
* exclusively on Microchip PICmicro Microcontroller products. The
* software is owned by the Company and/or its supplier, and is
* protected under applicable copyright laws. All rights are reserved.
* Any use in violation of the foregoing restrictions may subject the
* user to criminal sanctions under applicable laws, as well as to
* civil liability for the breach of the terms and conditions of this
* license.
*
* THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
* TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
* IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
* CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
*
*********************************************************************/

#include "p30fxxxx.h"
#include "system.h"
#include "user.h"

_FOSC(HS2_PLL16 & CSW_FSCM_OFF);
_FBORPOR(PWRT_OFF & BORV27 & PBOR_OFF & MCLR_EN);   /* Ext MCLR, no brownout */
_FWDT(WDT_OFF);                                     /* Turns off watchdog timer */

#define COMMAND_NACK     0x00
#define COMMAND_ACK      0x01
#define COMMAND_READ_PM  0x02
#define COMMAND_WRITE_PM 0x03
#define COMMAND_READ_EE  0x04
#define COMMAND_WRITE_EE 0x05
#define COMMAND_READ_CM  0x06
#define COMMAND_WRITE_CM 0x07
#define COMMAND_RESET    0x08
#define COMMAND_READ_ID  0x09

#define PM_ROW_SIZE 32
#define EE_ROW_SIZE 16
#define CM_ROW_SIZE 7
#define CONFIG_WORD_SIZE 1

#define PM_ROW_ERASE 		0x4041
#define PM_ROW_WRITE 		0x4001
#define EE_ROW_ERASE 		0x4045
#define EE_ROW_WRITE 		0x4005
#define CONFIG_WORD_WRITE	0X4008

/* Explicit declaration of memory.s file's assembly functions. */
void LoadAddr(unsigned int HW, unsigned int LW);
void WriteMem(unsigned int E);
void WriteLatch(unsigned int SourceAddrHW, unsigned int SourceAddrLW,
        unsigned int TempHW, unsigned int TempLW);

typedef short          Word16;
typedef unsigned short UWord16;
typedef long           Word32;
typedef unsigned long  UWord32;

typedef union tuReg32
{ UWord32 Val32;
    struct
    {
        UWord16 LW;
	UWord16 HW;
    } Word;
    char Val[4];
} uReg32;

extern UWord32 ReadLatch(UWord16, UWord16);

void PutChar(char);
void GetChar(char *);
void WriteBuffer(char *, int);
void ReadPM(char *, uReg32);
void ReadEE(char *, uReg32);
void WritePM(char *, uReg32);
void WriteEE(char *, uReg32);
void WriteCM(char *, uReg32);
void ResetDevice(void);

char Buffer[PM_ROW_SIZE*3 + 1];

/* usec delay program with T2 timer. */

void DelayuSec(long usec)
{   uReg32 Delay;
    Delay.Val32 = ((UWord32)(FCY/1000)) * ((UWord32)(usec));
    Delay.Val32 /= 1000;
    PR3 = Delay.Word.HW;
    PR2 = Delay.Word.LW;

    /* Enable Timer */
    T2CONbits.TON = 1;

    while (!IFS0bits.T3IF) ;
    IFS0bits.T3IF = 0;
    T2CONbits.TON = 0;
}

void FlashLED()
{
    LATEbits.LATE4 = 1;
    DelayuSec(2000);
    LATEbits.LATE4 = 0;
}

/******************************************************************************/
int main(void)
{
	uReg32 SourceAddr;
	uReg32 Delay;
	

        /* TODO FIXME the address !!! */
	SourceAddr.Val32 = 0x700;

        /* Initialize uart, and LED ports. */
        InitApp();

        /* Reading the delay value of first program word.*/
	Delay.Val32 = ReadLatch(SourceAddr.Word.HW, SourceAddr.Word.LW);

        /* Reset immediately, since delayval = 0; */
	if(Delay.Val[0] == 0)
	{
		ResetDevice();
	}

	T2CONbits.T32 = 1; /* to increment every instruction cycle */
	IFS0bits.T3IF = 0; /* Clear the Timer3 Interrupt Flag */
	IEC0bits.T3IE = 0; /* Disable Timer3 Interrup Service Routine */

	if((Delay.Val32 & 0x000000FF) != 0xFF)
	{
		/* Convert seconds into timer count value */
		Delay.Val32 = ((UWord32)(FCY)) * ((UWord32)(Delay.Val[0]));

		PR3 = Delay.Word.HW;
		PR2 = Delay.Word.LW;

		/* Enable Timer */
		T2CONbits.TON=1;
	}



        /* This is main program loop for intake bytes, and commands. .*/

	while(1)
	{
		char Command;

		GetChar(&Command);

		switch(Command)
		{
			case COMMAND_READ_PM:
			{
				uReg32 SourceAddr;

				GetChar(&(SourceAddr.Val[0]));
				GetChar(&(SourceAddr.Val[1]));
				GetChar(&(SourceAddr.Val[2]));

				ReadPM(Buffer, SourceAddr);

				WriteBuffer(Buffer, PM_ROW_SIZE*3);

				break;
			}

			case COMMAND_READ_EE:
			{
				uReg32 SourceAddr;

				GetChar(&(SourceAddr.Val[0]));
				GetChar(&(SourceAddr.Val[1]));
				GetChar(&(SourceAddr.Val[2]));

				ReadEE(Buffer, SourceAddr);

				WriteBuffer(Buffer, EE_ROW_SIZE*2);
			
				break;
			}


			case COMMAND_WRITE_PM:
			{
				uReg32 SourceAddr;
				int    Size;

				GetChar(&(SourceAddr.Val[0]));
				GetChar(&(SourceAddr.Val[1]));
				GetChar(&(SourceAddr.Val[2]));
				SourceAddr.Val[3]=0;
				
				for(Size = 0; Size < PM_ROW_SIZE*3; Size++)
				{
					GetChar(&(Buffer[Size]));
				}

				LoadAddr(SourceAddr.Word.HW,SourceAddr.Word.LW);   

				WriteMem(PM_ROW_ERASE);				/*Erase  page  */

				WritePM(Buffer, SourceAddr);		/*program page */

				PutChar(COMMAND_ACK);				/*Send Acknowledgement */

 				break;
			}

 			case COMMAND_WRITE_EE:
			{
				uReg32 SourceAddr;
				int    Size;

				GetChar(&(SourceAddr.Val[0]));
				GetChar(&(SourceAddr.Val[1]));
				GetChar(&(SourceAddr.Val[2]));
				
				for(Size = 0; Size < EE_ROW_SIZE*2; Size++)
				{
				GetChar(&(Buffer[Size]));
				}

				LoadAddr(SourceAddr.Word.HW,SourceAddr.Word.LW);
	
				WriteMem(EE_ROW_ERASE);
				
				WriteEE(Buffer, SourceAddr);

				PutChar(COMMAND_ACK);				/*Send Acknowledgement */

				break;
			}

			case COMMAND_WRITE_CM:	
			{
				int    Size;

				for(Size = 0; Size < CM_ROW_SIZE*3;)
				{
					GetChar(&(Buffer[Size++]));
					GetChar(&(Buffer[Size++]));
					GetChar(&(Buffer[Size++]));
				
					PutChar(COMMAND_ACK);				/*Send Acknowledgement */
				}

				
				break;
			}

			case COMMAND_RESET:
			{
				uReg32 SourceAddr;
				int    Size;
				uReg32 Temp;
				

				for(Size = 0, SourceAddr.Val32 = 0xF80000; Size < CM_ROW_SIZE*3; 
																Size +=3, SourceAddr.Val32 += 2)
				{
					if(Buffer[Size] == 0)
					{
						Temp.Val[0]=Buffer[Size+1];
						Temp.Val[1]=Buffer[Size+2];

						WriteLatch( SourceAddr.Word.HW,
										SourceAddr.Word.LW,
										Temp.Word.HW,
										Temp.Word.LW);

						WriteMem(CONFIG_WORD_WRITE);
					}
				}
				

				ResetDevice();

				break;
			}

			case COMMAND_NACK:
			{
				ResetDevice();

				break;
			}

			case COMMAND_READ_ID:
			{
				uReg32 SourceAddr;
				uReg32 Temp;

				SourceAddr.Val32 = 0xFF0000;

				Temp.Val32 = ReadLatch(SourceAddr.Word.HW, SourceAddr.Word.LW);

				WriteBuffer(&(Temp.Val[0]), 4);

				SourceAddr.Val32 = 0xFF0002;

				Temp.Val32 = ReadLatch(SourceAddr.Word.HW, SourceAddr.Word.LW);

				WriteBuffer(&(Temp.Val[0]), 4);

				break;
			}

		 	default:
				PutChar(COMMAND_NACK);
				break;
		}
	}

	return 0;
}
/******************************************************************************/
void ReadPM(char * ptrData, uReg32 SourceAddr)
{
	int    Size;
	uReg32 Temp;

	for(Size = 0; Size < PM_ROW_SIZE; Size++)
	{
		Temp.Val32 = ReadLatch(SourceAddr.Word.HW, SourceAddr.Word.LW);

		ptrData[0] = Temp.Val[2];;
		ptrData[1] = Temp.Val[1];;
		ptrData[2] = Temp.Val[0];;

		ptrData = ptrData + 3;

		SourceAddr.Val32 = SourceAddr.Val32 + 2;
	}
}
/******************************************************************************/

void ReadEE(char * ptrData, uReg32 SourceAddr)
{
	int    Size;
	uReg32 Temp;

	for(Size = 0; Size < EE_ROW_SIZE; Size++)
	{
		Temp.Val32 = ReadLatch(SourceAddr.Word.HW, SourceAddr.Word.LW);

		ptrData[0] = Temp.Val[1];
		ptrData[1] = Temp.Val[0];
		

		ptrData = ptrData + 2;

		SourceAddr.Val32 = SourceAddr.Val32 + 2;
	}
}

/******************************************************************************/
void WritePM(char * ptrData, uReg32 SourceAddr)
{
	int    Size,Size1;
	uReg32 Temp;

	for(Size = 0,Size1=0; Size < PM_ROW_SIZE; Size++)
	{
		
		Temp.Val[0]=ptrData[Size1+0];
		Temp.Val[1]=ptrData[Size1+1];
		Temp.Val[2]=ptrData[Size1+2];
		Temp.Val[3]=0;
		Size1+=3;
	    WriteLatch(SourceAddr.Word.HW, SourceAddr.Word.LW,Temp.Word.HW,Temp.Word.LW);

		SourceAddr.Val32 = SourceAddr.Val32 + 2;
	}

   WriteMem(PM_ROW_WRITE);
}
/******************************************************************************/
void WriteEE(char * ptrData, uReg32 SourceAddr)
{
	int    Size,Size1;
	uReg32 Temp;

	for(Size = 0,Size1=0; Size < EE_ROW_SIZE; Size++)
	{
		
		Temp.Val[0]=ptrData[Size1+0];
		Temp.Val[1]=ptrData[Size1+1];
		Temp.Val[2]=0;
		Temp.Val[3]=0;
		Size1+=2;
	    WriteLatch(SourceAddr.Word.HW, SourceAddr.Word.LW,Temp.Word.HW,Temp.Word.LW);

		SourceAddr.Val32 = SourceAddr.Val32 + 2;
	}
    WriteMem(EE_ROW_WRITE);
}
/******************************************************************************/

void WriteBuffer(char * ptrData, int Size)
{
	int DataCount;
	
	for(DataCount = 0; DataCount < Size; DataCount++)
	{
		PutChar(ptrData[DataCount]);
	}
}
/******************************************************************************/
void PutChar(char Char)
{
	while(!U1STAbits.TRMT);
	
	U1TXREG = Char;
}
/******************************************************************************/
void GetChar(char * ptrChar)
{
	while(1)
	{	
		/* if timer expired, signal to application to jump to user code */
		if(IFS0bits.T3IF == 1)
		{
			* ptrChar = COMMAND_NACK;
			break;
		}

		/* check for receive errors */
		if(U1STAbits.FERR == 1)
		{
			continue;
		}
			
		/* must clear the overrun error to keep uart receiving */
		if(U1STAbits.OERR == 1)
		{
			U1STAbits.OERR = 0;
			continue;
		}

		/* get the data */
		if(U1STAbits.URXDA == 1)
		{
			T2CONbits.TON=0; /* Disable timer countdown */

			* ptrChar = U1RXREG;
			break;
		}
	}
}
/******************************************************************************/


