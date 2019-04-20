#ifndef LAB3_SD_H
#define	LAB3_SD_H

#include <pic.h>
#include <stdbool.h>
#include "Lab3_CRC.h"
#include "Lab3_SPI.h"

/* ---------------------------- Global Variables ---------------------------- */

char GLBL_Resp8 = 0xFF;
char GLBL_Resp40[5] = {0xFF,0xFF,0xFF,0xFF,0xFF};
char GLBL_WriteBuffer[512];
char GLBL_ReadBuffer[512];

/* -------------------------- Function Prototypes --------------------------- */

#include "Lab3_SD.h"

//REQUIRES: SPI interface initialized using SPI_Init.
//PROMISES: Performs the SD Card initialization process over SPI:
//          1. Sets CS high and writes 0xFF for 80 cycles.
//          2. Send CMD0 with argument 0x00000000 until response is 0x01
//          3. Send CMD8 with argument 0x000001AA until response is 0x01000001AA
//          4a.Send CMD55 with argument 0x00000000 until response ix 0x00
//          4b.Send Send CMD41 with argument 0x40000000, if response isn't 0x00
//             go back to step 4a.
void SD_Init(void);

//REQUIRES: SPI interface initialized using SPI_Init.
//          Argument CMD_6bit is the command code from 0 to 64.
//          Arguments ARG3, ARG2, ARG1 and ARG0 are the four bytes of the
//          specific command's argument.
//PROMISES: Sends a 48-bit command to the SD card over SPI.
//          Calculates and sends the 7-bit checksum for the command.
void SD_SendCommand(char CMD_6bit, char ARG3, char ARG2, char ARG1, char ARG0);

//REQUIRES: SPI interface initialized using SPI_Init.
//PROMISES: For the SD card, 0xFF is 'no data'. Sends 0xFF to the device 
//          repeatedly until a response other than 0xFF is recieved. Then stores
//          the one-byte response in the global variable GLBL_Resp8.
void SD_Read8bitResponse(void);

//REQUIRES: Nothing.
//PROMISES: Compares the input argument Byte to the global variable GLBL_Resp8
//          Returns true (1) if they are equal, and false (0) otherwise.
bool SD_Check8bitResponse(char Byte);

//REQUIRES: SPI interface initialized using SPI_Init.
//PROMISES: For the SD card, 0xFF is 'no data'. Sends 0xFF to the device 
//          repeatedly until a response other than 0xFF is recieved. Then stores
//          the five-byte response in the global array GLBL_Resp40.
void SD_Read40bitResponse(void);

//REQUIRES: Nothing.
//PROMISES: Compares the input arguments Byte4 through Byte0 to the global 
//          variable GLBL_Resp8. Returns true if all comparisons are equal,
//          and false (0) otherwise.
bool SD_Check40bitResponse(char Byte4, char Byte3, char Byte2, char Byte1, char Byte0);

//REQUIRES: SPI interface initialized using SPI_Init.
//          SD Card initialized using SD_Init.
//          Global 512-byte array GLBL_WriteBuffer contains the data to write. 
//PROMISES: Writes the 512 bytes stored in GLBL_WriteBuffer to the SD card
//          at the address given by ADDR3-0.
//          Returns true if the write was successful, false otherwise.
bool SD_WriteBlock(char ADDR3, char ADDR2, char ADDR1, char ADDR0);

//REQUIRES: SPI interface initialized using SPI_Init.
//          SD Card initialized using SD_Init.
//          Global 512-byte array GLBL_WriteBuffer contains the data to write. 
//PROMISES: Reads the 512 bytes stored in the SD card block given by ADDR3-0
//          and saves them to the global array GLBL_ReadBuffer.
//          Returns true if the read was successful, false otherwise.
//          Does NOT verify the checksum of the read data.
bool SD_ReadBlock(char ADDR3, char ADDR2, char ADDR1, char ADDR0);
    

/* ------------------ #define based Function Declarations ------------------- */

//REQUIRES: SPI interface initialized using SPI_Init.
//PROMISES: Sets the Chip Select line for the SD Card high.
//          Clears any collision detected by the SPI interface.
#define SD_SET_CS_HIGH(){           \
    PORTCbits.RC7 = 1;              \
    SSP1CON1bits.WCOL = 0;          \
    }

//REQUIRES: SPI interface initialized using SPI_Init.
//PROMISES: Sets the Chip Select line for the SD Card low.
//          Clears any collision detected by the SPI interface.
#define SD_SET_CS_LOW(){            \
    PORTCbits.RC7 = 0;              \
    SSP1CON1bits.WCOL = 0;          \
    }

/* -------------------------------------------------------------------------- */

#endif	/* LAB3_SD_H */

