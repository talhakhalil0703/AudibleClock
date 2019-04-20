#ifndef LAB3_SPI_H
#define	LAB3_SPI_H

#include <pic.h>

/* ---------------------------- Global Variables ---------------------------- */



/* -------------------------- Function Prototypes --------------------------- */
// REQUIRES: Nothing.
// PROMISES: Configure SPI peripheral with RC2 = SCLK, RC3 = MISO, RC4 = MOSI
//           Configure pin RC7 as digital output (CS for SD Card)
//           Configure SPI Baud Rate to Fosc / 4
void SPI_Init(void);

// REQUIRES: SPI interface initialized using SPI_Init.
//           Argument Data_8bit is one byte of data to send over SPI.
// PROMISES: Transmits Data_8bit on the MOSI line.
void SPI_Write(char Data_8bit);

// REQUIRES: SPI interface initialized using SPI_Init.
// PROMISES: Transmits the byte 0xFF on the MOSI line.
//           Returns one byte of data recieved on the MISO line.
char SPI_Read(void);

/* ------------------ #define based Function Declarations ------------------- */


/* -------------------------------------------------------------------------- */

#endif	/* LAB3_SPI_H */

