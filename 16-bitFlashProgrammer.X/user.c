/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#include <p30Fxxxx.h>        /* Device header file                            */
#include <stdint.h>          /* For uint32_t definition                       */
#include <stdbool.h>         /* For true/false definition                     */

#include "user.h"            /* variables/params used by user.c               */
#include "system.h"

/******************************************************************************/
/* User Functions                                                             */
/******************************************************************************/

/* <Initialize variables in user.h and insert code for user algorithms.> */
void UART1Conf()
{
    U1MODEbits.UARTEN = 0;  // Disable UART1
    U1RXD_TRIS = 1;     // U1RXD set in input.
    U1TXD_TRIS = 0;     // U1TXD set output.
    U1RTS_TRIS = 0;     // U1RTS port set output.
    U1MODEbits.ALTIO = 0;   // The U1TX, and U1RX port are used by UART module.

    U1MODEbits.PDSEL = 0b00;    // 8 bit, no parity
    U1MODEbits.STSEL = 0b0;     // 1 stop bit enought
    /* The default (power-on) setting of the UART is 8 bits, no parity,
     * 1 Stop bit (typically represented as 8, N, 1). */

    /* Interrupt flag bit is set when a character is received */
    U1STAbits.URXISEL = 0b00; // @TODO WARNING

    /*ADDEN: Address Character Detect (bit 8 of received data = 1)
     * 1 = Address Detect mode enabled. If 9-bit mode is not selected, this
     * control bit has no effect. 0 = Address Detect mode disabled */
    U1STAbits.ADDEN = 0b0;

    /* (FCY/(16*BAUD))-1*/
    U1BRG = BRGVAL;
            // CAL_BRG(2400);

    U1MODEbits.UARTEN = 1; // @TODO WARNING Enable UART1 communication.

        /* UART transmitter enabled, UxTX pin controlled by UART (if UARTEN = 1)*/
    U1STAbits.UTXEN = 1;

}

/* TODO Initialize User Ports/Peripherals/Project here */

void InitApp(void)
{
    /* Configure UART, to download the program. */
    UART1Conf();

    /* Port setup for LED blinking due download the program... */
    TRISC |= 0b0110000000000000;
    TRISE |= 0b0000000100110000;

    TRISEbits.TRISE4 = 0;
    TRISCbits.TRISC14 = 0;
    LATE = 0;
    LATC = 0;

}

