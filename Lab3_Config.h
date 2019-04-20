#ifndef CONFIGURATION_H
#define	CONFIGURATION_H

#define _XTAL_FREQ 32000000
#pragma config FOSC = INTOSC    // Oscillator Selection Bits->INTOSC oscillator: I/O function on CLKIN pin
#pragma config WDTE = OFF    // Watchdog Timer Enable->WDT disabled
#pragma config PLLEN = ON    // Phase Lock Loop enable->4x PLL is always enabled

#endif	/* CONFIGURATION_H */

