/******************************************************************************/
/* User Level #define Macros                                                  */
/******************************************************************************/

/* <Initialize variables in user.h and insert code for user algorithms.>
 * These are the LED display's in/out lines.*/

#define CAL_BRG(BAUD_RATE) ((FCY/(16*BAUD_RATE))-1)
#define BAUDRATE 19200l
#define BRGVAL          ((FCY/BAUDRATE)/16)-1

#define LED_BLINKING

/*---- RS485 input section */
    // U1RXD_TRIS register. Then must be set input.
#define U1RXD_TRIS  TRISFbits.TRISF2    
    // U1RXD_PORT register. Read this port to reading
#define U1RXD_PORT  PORTFbits.RF2
/* --------- RS485 output section(s)*/
    // U1TXD_TRIS register. Then must be set output.
#define U1TXD_TRIS  TRISFbits.TRISF3
    /* U1TXD_LAT for test mode ...*/
#define U1TXD_LAT   LATFbits.LATF3
/* ------------------------------------------------------ */
    /* U1RTS_TRIS register. That must set output, because this is data transmit
     * enabled bit of RS485 insulated signal.*/
#define U1RTS_TRIS  TRISFbits.TRISF5

    /* This is U1 RTS signal for RS485 line. If enabled this line, the RS485
     * channel can data transmit, therefore the line goes from high 
     * impedance mode to normal data transmit mode. Since the RS 485 line 
     * use only some data connection to data read, and data transmit, 
     * (half duplex mode) the data line we must released when we can it as is time.
     * If U1RTS_LAT=0, then the line connect to rail (transmitting mode),
     * and if U1RTS_LAT=1, then the RS485 port became high impedance. (Receiving mode)*/
#define U1RTS_LAT LATFbits.LATF5

/* These are datas to TRISX registers to input state with the or expressions the
 * input TRISE datas.
 */


void InitApp(void); /* I/O and Peripheral Initialization */
