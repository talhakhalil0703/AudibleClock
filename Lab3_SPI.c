#include "Lab3_SPI.h"

// REQUIRES: Nothing.
// PROMISES: Configure SPI peripheral with RC2 = SCLK, RC3 = MISO, RC4 = MOSI
//           Configure pin RC7 as digital output (CS for SD Card)
//           Configure SPI Baud Rate to Fosc / 4
void SPI_Init(void)
{
    //Configure Pin C2 as Serial Clock
    TRISC2 = 0;         //RC2 = Output
    RC2PPS = 0x21;      //RC2 = SCK
    SSPCLKPPS = 0x12;   //RC2 = SCK
    
    //Configure Pin C3 as Serial Data In
    TRISC3 = 1;         //RC3 = Input
    ANSC3 = 0;          //RC3 = Digital
    SSPDATPPS = 0x13;   //RC3 = SDI; 
    
    //Configure Pin C4 as Serial Data Out
    TRISC4 = 0;         //RC4 = Output
    RC4PPS = 0x23;      //RC4 = SDO
    
    //Configure Pin C7 as Digital Output, for use as Flash Chip Select
    TRISC7 = 0;         //RC7 = Output
    
    //SSP1STAT: SSP STATUS REGISTER                 (User Guide page 488)
    SSP1STAT = 0x80;
    
    //SSP1CON1: SSP CONTROL REGISTER 1              (User Guide page 489)
    SSP1CON1 = 0x30;
    
    //SSP1ADD: MSSP ADDRESS AND BAUD RATE REGISTER  (User Guide page 492)
    SSP1ADD = 0x00;
}

// REQUIRES: SPI interface initialized using SPI_Init.
//           Argument Data_8bit is one byte of data to send over SPI.
// PROMISES: Transmits Data_8bit on the MOSI line.
void SPI_Write(char Data_8bit)
{
    SSP1BUF = Data_8bit;
    while(SSP1STATbits.BF == 0);
}

// REQUIRES: SPI interface initialized using SPI_Init.
// PROMISES: Transmits the byte 0xFF on the MOSI line.
//           Returns one byte of data recieved on the MISO line.
char SPI_Read(void)
{
    SSP1BUF = 0xFF;
    while(SSP1STATbits.BF == 0);  
    return SSP1BUF;
}
