#include <pic.h>
#include "Lab3_Config.h"
#include "Lab3_SPI.h"
#include "Lab3_SD.h"

/* Audio filesystem array to sound
 0  am          1  eight        2  eighteen     3  eleven       4  fifteen          5  fifty
 6  five        7  four         8  fourteen     9  forty       10  nine            11  nineteen
 12  oeight     13  ofive       14  ofour       15  one         16  onine           17  oone
 18  oseven     19  osix        20  othree      21  otwo        22  pm              23  seven
 24  seventeen  25  six         26  sixteen     27  ten         28  thirteen        29  thirty
 30  three      31  twelve      32  twenty      33  two         34  alarm           35  time
 36  set        37  muffin      38  alarmNoise

 */

//global variables that are used to read SD card files in increments
short G_8counter = 0;
unsigned char * G_u8charPointer;
unsigned short G_u16Address;
unsigned short G_u16EndAddress;
unsigned short G_u16hour = 13;
unsigned short G_u16min = 0;
unsigned short G_u16sec = 0;
unsigned short G_u16alarmHour = 13;// for Alarm
unsigned short G_u16alarmMin = 1;
unsigned long G_u32timeTrack = 0;
unsigned char G_u8play = 0x00;
char G_8sendbyte; // the byte that will be sent to DAQ
char G_8hasAddress = 0;
bool timeSaid = false;
bool setAlarm = true;
bool pressedTalk = false;
bool pressedHour = false;
bool pressedMin = false;
bool pressedToggle = false;
bool pressedSnooze = false;
bool snooze = false;
unsigned short G_au16startAddresses[] = {0x0000, 0x0017, 0x0036, 0x005a, 0x007c, 0x03B0, 0x00ae, 0x00c2, 0x00d8, //Address values of audio files
                                         0x00fc, 0x010c, 0x0122, 0x014f, 0x0168, 0x018c, 0x01ac, 0x01c1, 0x01dc,
                                         0x01f7, 0x0215, 0x0234, 0x0250, 0x026f, 0x0289, 0x029c, 0x02c1, 0x02d6,
                                         0x02fd, 0x0319, 0x033a, 0x034e, 0x0360, 0x0384, 0x039c, 0x03CB, 0x03E9,
                                         0x0402, 0x0415, 0x1B6C};
unsigned short G_au16endAddresses[] = {0x0017, 0x0036, 0x005a, 0x007c, 0x00a0, 0x03CB, 0x00c2, 0x00d8, 0x00fc,
                                        0x010c, 0x0122, 0x014f, 0x0168, 0x018c, 0x01ac, 0x01c1, 0x01dc, 0x01f7,
                                        0x0215, 0x0234, 0x0250, 0x026f, 0x0289, 0x029c, 0x02c1, 0x02d6, 0x02fd,
                                        0x0319, 0x033a, 0x034e, 0x0360, 0x0384, 0x039c, 0x03B0, 0x03E9, 0x0402,
                                        0x0415, 0x1B6C, 0x223E};

//function prototypes
void writeDAQ (char writevalue); //Writes to DAQ
void playAudio (short s, short e); // e for end, s for start this the address arguments which will have to be given manually
void active(void); //reading the bytes
void readTime(short h, short m);
void playAlarm(void); //Plays the alarm
void playHour (short h);
void playMinute10 (short m);
void playMinute (short m);
void configButtons(void); // configures all the buttons for their use
void checkButtonPress(void); // Checks for button Press
__interrupt() void timeInterrupt(void); // Increments time, and rolls the time numbers

void main(void) {

    // Set the system clock speed to 32MHz and wait for the ready flag.
    OSCCON = 0xF4;
    while(OSCSTATbits.HFIOFR == 0);
    //Initialize all required peripherals.
    SPI_Init();
    SD_Init();
    configButtons();

    DAC1CON0 = 0xA0;
    T2CLKCON = 0x05; //setting timer 2 clock to 16khz
    T2PR = 0x1f;
    T2CON = 0x80;

    INTCON = 0xC0;// Enables PEIE and GIE
    PIE1 = 0x01; // Enables timer 1 overflow
    T1CON = 0xC3; //0xF1;// uses LFintosc, prescales by 1, and turns timer 1 on.
    TMR1IF = false;

    while(1)
    {
    checkButtonPress();// Button Functionality
    playAlarm(); //checks for the alarm condition and then plays the alarm if true
    readTime(G_u16hour, G_u16min); //Only reads based on button press
    }
    return;
}

void configButtons()
{
    TRISBbits.TRISB4 = 1; //Configuring RB4 as Talk Time
    ANSELBbits.ANSB4 = 0;
    TRISBbits.TRISB3 = 1; //Configuring RB3 as increment hour
    ANSELBbits.ANSB3 = 0;
    TRISBbits.TRISB2 = 1; //Configuring RB2 as increment min
    ANSELBbits.ANSB2 = 0;
    TRISBbits.TRISB1 = 1; //Configuring RB1 as toggle Alarm/Time set
    ANSELBbits.ANSB1 = 0;
    TRISBbits.TRISB0 = 1; //Configuring RB0 as Snooze Alarm
    ANSELBbits.ANSB0 = 0;
    TRISBbits.TRISB5 = 0; //Configuring RB0 as Snooze Alarm
    ANSELBbits.ANSB5 = 0;
    return;
}

void checkButtonPress()
{
    //Talk Time
        if (PORTBbits.RB4 == 1 && !pressedTalk)
        {
            pressedTalk = true;
        }
        if (PORTBbits.RB4 == 0 && pressedTalk)
        {
            pressedTalk = false;
            timeSaid = true;
        }

        //Hour Increment
        if (PORTBbits.RB3 == 1 && !pressedHour)
        {
            pressedHour = true;
        }
        if (PORTBbits.RB3 == 0 && pressedHour)
        {
            pressedHour = false;
            if (setAlarm)
            {
                G_u16alarmHour++;
                if (G_u16alarmHour == 24)
                {
                    G_u16alarmHour  = 0;
                }
                timeSaid = true;
                readTime(G_u16alarmHour, G_u16alarmMin);
            } else {
                G_u16hour++;
                if (G_u16hour == 24)
                {
                    G_u16hour  = 0;
                }
                timeSaid = true;
                readTime(G_u16hour, G_u16min);//to say adjusted time and not incremented time
            }
        }

        //Min Increment
        if (PORTBbits.RB2 == 1 && !pressedMin)
        {
            pressedMin = true;
        }
        if (PORTBbits.RB2 == 0 && pressedMin)
        {
           pressedMin = false;
           if (setAlarm)
            {
                G_u16alarmMin++;
                if (G_u16alarmMin  == 60)
                {
                    G_u16alarmMin  = 0;
                }
                timeSaid = true;
                readTime(G_u16alarmHour, G_u16alarmMin);
            } else {
                G_u16min++;
                if (G_u16min == 60)
                {
                    G_u16min  = 0;
                }
                timeSaid = true;
                readTime(G_u16hour, G_u16min);//to say adjusted time and not incremented time
            }
        }

        //Toggle Time/Alarm set
        if (PORTBbits.RB1 == 1 && !pressedToggle)
        {
            pressedToggle = true;
        }
        if (PORTBbits.RB1 == 0 && pressedToggle)
        {
            pressedToggle = false;
            if (setAlarm)
            {
                setAlarm = false;
                timeSaid = true;
                playAudio(G_au16startAddresses[35],G_au16endAddresses[35] );
                playAudio(G_au16startAddresses[36],G_au16endAddresses[36] );
                timeSaid = false;
            }
            else
            {
                setAlarm = true;
                timeSaid= true;
                playAudio(G_au16startAddresses[34],G_au16endAddresses[34] );
                playAudio(G_au16startAddresses[36],G_au16endAddresses[36] );
                timeSaid = false;
            }
        }

         //Snooze << should be checked in active, being checked there....

}

void readTime (short hour, short minute) // plays time
{
    if (timeSaid == true)
    {
        if (hour < 12)
        { //am code
            playHour(hour);
            playMinute10(minute);
            playAudio(G_au16startAddresses[0], G_au16endAddresses[0]); //Saying Am at the end
        } else
        { //pm code
            unsigned short temphour = hour - 12;
            playHour(temphour);
            playMinute10(minute);
            playAudio(G_au16startAddresses[22], G_au16endAddresses[22]); // Saying Pm at the end
        }
        timeSaid = false; //We shall wait for our button press
    }
    return;
}
void playAlarm() //plays the alarm sound if the alarm time is equal to the system time
{
    if (G_u16hour == G_u16alarmHour && G_u16min == G_u16alarmMin && !snooze)
    {
        timeSaid= true;
        playAudio(G_au16startAddresses[37],G_au16endAddresses[37]);
        timeSaid = false;
        return;
    }
    return;
}
void playHour (short hour) {
        if (hour == 0) { //play 12
            playAudio(G_au16startAddresses[31], G_au16endAddresses[31]);
        } else if (hour == 11) { //play 11
            playAudio(G_au16startAddresses[3], G_au16endAddresses[3]);
        } else if (hour == 10) { //play 10
            playAudio(G_au16startAddresses[27], G_au16endAddresses[27]);
        } else  if (hour == 9) { //play 9
            playAudio(G_au16startAddresses[10], G_au16endAddresses[10]);
        } else if (hour == 8) { //play 8
            playAudio(G_au16startAddresses[1], G_au16endAddresses[1]);
        } else if (hour == 7) { //play 7
            playAudio(G_au16startAddresses[23], G_au16endAddresses[23]);
        } else if (hour == 6) { //play 6
            playAudio(G_au16startAddresses[25], G_au16endAddresses[25]);
        } else if (hour == 5) { //play 5
            playAudio(G_au16startAddresses[6], G_au16endAddresses[6]);
        } else if (hour == 4) { //play 4
            playAudio(G_au16startAddresses[7], G_au16endAddresses[7]);
        } else if (hour == 3) { //play 3
            playAudio(G_au16startAddresses[30], G_au16endAddresses[30]);
        } else if (hour == 2) { //play 2
            playAudio(G_au16startAddresses[33], G_au16endAddresses[33]);
        } else if (hour == 1) { //play 1
            playAudio(G_au16startAddresses[15], G_au16endAddresses[15]);
        }
        return;
} //plays hour chunk

void playMinute10 (short minute) //play minute10's spot chunk
{
    if (minute/10 == 2){
           playAudio(G_au16startAddresses[32], G_au16endAddresses[32]); // Saying 20
    } else if (minute/10 == 3){
           playAudio(G_au16startAddresses[29], G_au16endAddresses[29]); // Saying 30
    } else if (minute/10 == 4){
           playAudio(G_au16startAddresses[9], G_au16endAddresses[9]); // Saying 40
    } else if (minute/10 == 5){
           playAudio(G_au16startAddresses[5], G_au16endAddresses[5]); // Saying 50
    }
     playMinute(minute);
    return;
}

void playMinute (short minute) //play minute chunk
{
    if (minute/10 == 1) {
        if (minute == 10) { //play 10
            playAudio(G_au16startAddresses[27], G_au16endAddresses[27]);
        } else  if (minute == 11) { //play 11
            playAudio(G_au16startAddresses[3], G_au16endAddresses[3]);
        } else if (minute == 12) { //play 12
            playAudio(G_au16startAddresses[31], G_au16endAddresses[31]);
        } else if (minute == 13) { //play 13
            playAudio(G_au16startAddresses[28], G_au16endAddresses[28]);
        } else if (minute == 14) { //play 14
            playAudio(G_au16startAddresses[8], G_au16endAddresses[8]);
        } else if (minute == 15) { //play 15
            playAudio(G_au16startAddresses[4], G_au16endAddresses[4]);
        } else if (minute == 16) { //play 16
            playAudio(G_au16startAddresses[26], G_au16endAddresses[26]);
        } else if (minute == 17) { //play 17
            playAudio(G_au16startAddresses[24], G_au16endAddresses[24]);
        } else if (minute == 18) { //play 18
            playAudio(G_au16startAddresses[2], G_au16endAddresses[2]);
        } else if (minute == 19) { //play 19
            playAudio(G_au16startAddresses[11], G_au16endAddresses[11]);
        }
    } else {
        if (minute == 29 || minute == 39 || minute == 49 || minute == 59) { //play 9
            playAudio(G_au16startAddresses[10], G_au16endAddresses[10]);
        } else if ( minute == 28 || minute == 38 || minute == 48 || minute == 58) { //play 8
            playAudio(G_au16startAddresses[1], G_au16endAddresses[1]);
        } else if ( minute == 27 || minute == 37 || minute == 47 || minute == 57) { //play 7
            playAudio(G_au16startAddresses[23], G_au16endAddresses[23]);
        } else if (minute == 26 || minute == 36 || minute == 46 || minute == 56) { //play 6
            playAudio(G_au16startAddresses[25], G_au16endAddresses[25]);
        } else if (minute == 25 || minute == 35 || minute == 45 || minute == 55) { //play 5
            playAudio(G_au16startAddresses[6], G_au16endAddresses[6]);
        } else if (minute == 24 || minute == 34 || minute == 44 || minute == 54) { //play 4
            playAudio(G_au16startAddresses[7], G_au16endAddresses[7]);
        } else if ( minute == 23 || minute == 33 || minute == 43 || minute == 53) { //play 3
            playAudio(G_au16startAddresses[30], G_au16endAddresses[30]);
        } else if (minute == 22 || minute == 32 || minute == 42 || minute == 52) { //play 2
            playAudio(G_au16startAddresses[33], G_au16endAddresses[33]);
        } else if (minute == 21 || minute == 31 || minute == 41 || minute == 51) { //play 1
            playAudio(G_au16startAddresses[15], G_au16endAddresses[15]);
        } else if (minute == 9) { //play 9
            playAudio(G_au16startAddresses[16], G_au16endAddresses[16]);
        } else if (minute == 8) { //play 8
            playAudio(G_au16startAddresses[12], G_au16endAddresses[12]);
        } else if (minute == 7) { //play 7
            playAudio(G_au16startAddresses[18], G_au16endAddresses[18]);
        } else if (minute == 6) { //play 6
            playAudio(G_au16startAddresses[19], G_au16endAddresses[19]);
        } else if (minute == 5) { //play 5
            playAudio(G_au16startAddresses[13], G_au16endAddresses[13]);
        } else if (minute == 4) { //play 4
            playAudio(G_au16startAddresses[14], G_au16endAddresses[14]);
        } else if (minute == 3) { //play 3
            playAudio(G_au16startAddresses[20], G_au16endAddresses[20]);
        } else if (minute == 2) { //play 2
            playAudio(G_au16startAddresses[21], G_au16endAddresses[21]);
        } else if (minute == 1) { //play 1
            playAudio(G_au16startAddresses[17], G_au16endAddresses[17]);
        }
    }
    return;
}

void playAudio(short startAddress, short endAddress) //This function sets the Address as in calls Active
{
    if (G_8hasAddress == 0 && timeSaid)
    {
    G_u16Address = startAddress; //takes the starting address and divides it by 512, and then stores it a long, this long is used to increment the segment you'll be reading
    G_u8charPointer = (unsigned char *) &G_u16Address;//pointer used to convert the long into char segments, these are what are sent to the SendCommand
    G_u16EndAddress = endAddress; // The address at which you need to stop reading at
    G_8hasAddress = 1; // Checks if you have the address already as to not reset at what segment point you are at.
    active();
    }
     return;
}

void active() //This function does the writing to the DAQ
{
    while (G_u16Address != G_u16EndAddress) {
        if (G_8counter == 0) { // checking if you need a new chunk of 512 bytes to read
            char readMessage = 0xFF; //Block read code, identical to the read block function
            SD_SendCommand(17, 0x00, 0x00, G_u8charPointer[1], G_u8charPointer[0]); // sending the read command
            SD_Read8bitResponse();
            do {readMessage = SPI_Read();}
            while (readMessage == 0xFF);
        }

        G_8sendbyte = SPI_Read(); // reading one byte at a time
        writeDAQ(G_8sendbyte); // sending to DAQ which converts the digital signal into analog and that is sent to the speaker
        G_8counter++; // counting how many bytes in the 512 byte chunk have been read so far

        if (G_8counter==512) { // if at the end of the chunk, increment the address and handle any overflow.
         G_u16Address++; //incrementing what you have read, changes the char values sent to sendCommand because they are updated from a pointer.
         G_8counter = 0; // if you're at the end of the chunk reset the counter
         SPI_Read();
         SPI_Read();
         SPI_Read();
        }
        while(!PIR1bits.TMR2IF); // waiting for the timer2 clock
        PIR1bits.TMR2IF = 0;// Can change timer2 to make it sound demonic if we want...

        if (PORTBbits.RB0 == 1 && !pressedSnooze) //checking for Snooze
        {
            pressedSnooze = true;
        }
        if (PORTBbits.RB0 == 0 && pressedSnooze) //Basically stops the active call, won't work beacause it only checks after playing the sound..
        {
                pressedSnooze = false;
                timeSaid = false;
                G_8hasAddress = 0;
                snooze = true;
                return;
        }

    }
    G_8hasAddress = 0;
    return;
}

void writeDAQ (char writevalue) { //Digital to analog converter. Using the higher end of DAC because we want the speaker to be louder.
  DAC1REF= writevalue;
  DAC1LD = 1;
}

__interrupt() void timeInterrupt()
{
    static unsigned short G_u16secCheck = 0;

    G_u16sec += 2;
    G_u16secCheck += 2;
    if (G_u16secCheck == 100)
    {
        G_u16sec -= 1;
        G_u16secCheck = 0;
    }

    if (G_u16sec == 60 || G_u16sec == 59)
     {
         G_u16min++;
         snooze = false;
         if (G_u16min == 60)
         {
             G_u16min = 0;
             G_u16hour++;
             if (G_u16hour == 24)
             {
             G_u16hour  = 0;
             }
         }
         G_u16sec = 0;
     }
    TMR1IF = false;
 }
