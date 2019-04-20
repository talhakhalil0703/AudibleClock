#include "Lab3_SD.h"

//REQUIRES: SPI interface initialized using SPI_Init.
//PROMISES: Performs the SD Card initialization process over SPI:
//          1. Sets CS high and writes 0xFF for 80 cycles.
//          2. Send CMD0 with argument 0x00000000 until response is 0x01
//          3. Send CMD8 with argument 0x000001AA until response is 0x01000001AA
//          4a.Send CMD55 with argument 0x00000000 until response ix 0x00
//          4b.Send Send CMD41 with argument 0x40000000, if response isn't 0x00
//             go back to step 4a.
void SD_Init(void)
{
    //Step 1:
    //Set Chip Select high, and write 0xFF for at least 74 cycles.
    SD_SET_CS_HIGH();
    
    SPI_Write(0xFF);
    SPI_Write(0xFF);
    SPI_Write(0xFF);
    SPI_Write(0xFF);
    SPI_Write(0xFF);
    SPI_Write(0xFF);
    SPI_Write(0xFF);
    SPI_Write(0xFF);
    SPI_Write(0xFF);
    SPI_Write(0xFF);
    
    SD_SET_CS_LOW();

    //Step 2:
    //Send CMD0 with argument 0x00000000.
    //Expect 8-bit response 0x01.
    //On any other response, retry CMD0.
    do {
        SD_SendCommand(0, 0x00, 0x00, 0x00, 0x00);
        SD_Read8bitResponse();
        asm("NOP");
    } while (SD_Check8bitResponse(0x01) == false);
    
    //Step 3:
    //Send CMD8 with argument 0x000001AA.
    //Expect 40-bit response 0x01000001AA.
    //On any other response, retry CMD8.
    do {
        SD_SendCommand(8, 0x00, 0x00, 0x01, 0xAA);
        SD_Read40bitResponse();
        asm("NOP");
    } while (SD_Check40bitResponse(0x01, 0x00, 0x00, 0x01, 0xAA) == false);

    //Step 4a
    //Send CMD55 with argument 0x00000000.
    //Expect 8-bit response 0x01.
    //On any other response, retry CMD55.
    do {
        do {
            SD_SendCommand(55, 0x00, 0x00, 0x00, 0x00);
            SD_Read8bitResponse();
            asm("NOP");
        } while (SD_Check8bitResponse(0x01) == false);
    
        //Step 4b:
        //If the CMD55 response is good,
        //Send CMD41 with argument 0x40000000
        //Expect 8-bit response 0x00
        //On any other response, go back to CMD55.
        SD_SendCommand(41, 0x40, 0x00, 0x00, 0x00);
        SD_Read8bitResponse();
        asm("NOP");
    } while (SD_Check8bitResponse(0x00) == false);
}

//REQUIRES: SPI interface initialized using SPI_Init.
//          Argument CMD_6bit is the command code from 0 to 64.
//          Arguments ARG3, ARG2, ARG1 and ARG0 are the four bytes of the
//          specific command's argument.
//PROMISES: Sends a 48-bit command to the SD card over SPI.
//          Calculates and sends the 7-bit checksum for the command.
void SD_SendCommand(char CMD_6bit, char ARG3, char ARG2, char ARG1, char ARG0)
{
    char ByteToSend = 0xFF;
    char Checksum = 0x00;
    
    //First byte is 01XXXXXX, where X are the 6 command bits.
    ByteToSend = (CMD_6bit & 0x3F) | 0x40;
    SPI_Write(ByteToSend);
    UPDATE_CRC(Checksum, ByteToSend);
     
    //Next 4 bytes are the 32 argument bits.
    ByteToSend = ARG3;
    SPI_Write(ByteToSend);
    UPDATE_CRC(Checksum, ByteToSend);
    
    ByteToSend = ARG2;
    SPI_Write(ByteToSend);
    UPDATE_CRC(Checksum, ByteToSend);
    
    ByteToSend = ARG1;
    SPI_Write(ByteToSend);
    UPDATE_CRC(Checksum, ByteToSend);
    
    ByteToSend = ARG0;
    SPI_Write(ByteToSend);
    UPDATE_CRC(Checksum, ByteToSend);
    
    //Last byte is XXXXXXX1, where X are the 7 Cyclic Redundancy Check bits.
    ByteToSend = (Checksum << 1) | 0x01;
    SPI_Write(ByteToSend);
}

//REQUIRES: SPI interface initialized using SPI_Init.
//PROMISES: For the SD card, 0xFF is 'no data'. Sends 0xFF to the device 
//          repeatedly until a response other than 0xFF is recieved. Then stores
//          the one-byte response in the global variable GLBL_Resp8.
void SD_Read8bitResponse(void)
{
    char readMessage = 0xFF;
    
    //We don't know when the response will come, but the first bit is always
    //a zero. Read bytes until the response contains at least one zero.
    do {readMessage = SPI_Read();} 
    while (readMessage == 0xFF);   
    
    //Read the one message bytes into the global variable
    GLBL_Resp8 = readMessage;
    
    //Read once more and ignore the result.
    SPI_Read();
        
    return;
}

//REQUIRES: Nothing.
//PROMISES: Compares the input argument Byte to the global variable GLBL_Resp8
//          Returns true (1) if they are equal, and false (0) otherwise.
bool SD_Check8bitResponse(char Byte)
{
    bool match = true;
    if (Byte != GLBL_Resp8) match = false;
    return match;
}
//REQUIRES: SPI interface initialized using SPI_Init.
//PROMISES: For the SD card, 0xFF is 'no data'. Sends 0xFF to the device 
//          repeatedly until a response other than 0xFF is recieved. Then stores
//          the five-byte response in the global array GLBL_Resp40.
void SD_Read40bitResponse(void)
{
    char readMessage = 0xFF;
    
    //We don't know when the response will come, but the first bit is always
    //a zero. Read bytes until the response contains at least one zero.
    do {readMessage = SPI_Read();} 
    while (readMessage == 0xFF);   
    
    //Read the five message bytes into the global array
    GLBL_Resp40[0] = readMessage;
    GLBL_Resp40[1] = SPI_Read();
    GLBL_Resp40[2] = SPI_Read();
    GLBL_Resp40[3] = SPI_Read();
    GLBL_Resp40[4] = SPI_Read();
    
    //Read once more and ignore the result.
    SPI_Read();
    
    return;
}

//REQUIRES: Nothing.
//PROMISES: Compares the input arguments Byte4 through Byte0 to the global 
//          variable GLBL_Resp8. Returns true if all comparisons are equal,
//          and false (0) otherwise.
bool SD_Check40bitResponse(char Byte4, char Byte3, char Byte2, char Byte1, char Byte0)
{
    bool match = true;
    if (Byte4 != GLBL_Resp40[0]) match = false;
    if (Byte3 != GLBL_Resp40[1]) match = false;
    if (Byte2 != GLBL_Resp40[2]) match = false;
    if (Byte1 != GLBL_Resp40[3]) match = false;
    if (Byte0 != GLBL_Resp40[4]) match = false;    
    return match;
}

//REQUIRES: SPI interface initialized using SPI_Init.
//          SD Card initialized using SD_Init.
//          Global 512-byte array GLBL_WriteBuffer contains the data to write. 
//PROMISES: Writes the 512 bytes stored in GLBL_WriteBuffer to the SD card
//          at the address given by ADDR3-0.
//          Returns true if the write was successful, false otherwise.
bool SD_WriteBlock(char ADDR3, char ADDR2, char ADDR1, char ADDR0)
{
    //Send the block write command to the SD card
    SD_SendCommand(24, ADDR3, ADDR2, ADDR1, ADDR0);
    
    //If the response is anything but 0x00, we cannot write.
    SD_Read8bitResponse();
    if(SD_Check8bitResponse(0x00) == false) return false;
    
    //Write a few empty cycles to give the SD card time.
    SPI_Write(0xFF);
    SPI_Write(0xFF);
    
    //Write the Data Token to signify the start of the packet
    SPI_Write(0xFE);
    
    //Write the contents of the block write buffer.
    for(int i = 0; i < 512; i++)
    {
        SPI_Write(GLBL_WriteBuffer[i]);
    }
    
    //Read the Data Response byte
    SD_Read8bitResponse();
    
    //Check the data response byte. We expect 0xE5 on a successful write.
    if(SD_Check8bitResponse(0xE5) == false) return false;
    
    //Write was a success, so return true.
    return true;
}

//REQUIRES: SPI interface initialized using SPI_Init.
//          SD Card initialized using SD_Init.
//          Global 512-byte array GLBL_WriteBuffer contains the data to write. 
//PROMISES: Reads the 512 bytes stored in the SD card block given by ADDR3-0
//          and saves them to the global array GLBL_ReadBuffer.
//          Returns true if the read was successful, false otherwise.
//          Does NOT verify the checksum of the read data.
bool SD_ReadBlock(char ADDR3, char ADDR2, char ADDR1, char ADDR0)
{
    char readMessage = 0xFF;
    
    //Send the block read command (CMD17) to the SD card.
    //The 32 bit argument is which 512-byte sector to read.
    SD_SendCommand(17, ADDR3, ADDR2, ADDR1, ADDR0);
    
    //Wait for the SD card to respond to the command.
    SD_Read8bitResponse();
    
    //If the response is anything but 0x00, we cannot read.
    if(SD_Check8bitResponse(0x00) == false) return false;
    
    //We don't know when the SD card will start sending data, but the first byte
    //is always 0xFE. Read bytes until the response contains at least one zero.
    do {readMessage = SPI_Read();} 
    while (readMessage == 0xFF);   

    //If the message is anything but 0xFE, we cannot read.
    if (readMessage != 0xFE) return false;
    
    //Start reading bytes into the read buffer.
    for(int i = 0; i < 512; i++)
    {
        GLBL_ReadBuffer[i] = SPI_Read();
    }
    
    //The next two bytes are the block's 16-bit CRC. We won't worry about it.
    SPI_Read();
    SPI_Read();
        
    SPI_Read();
    
    //Read was a success, so return true.
    return true;
}
    