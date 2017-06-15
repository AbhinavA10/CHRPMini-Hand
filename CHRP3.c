/*==============================================================================
	CHRP 3.1 (PIC18F25K50) hardware initialization and user functions.
==============================================================================*/

#include	"xc.h"				// XC compiler general include file

#include	"stdint.h"			// Include integer definitions
#include	"stdbool.h"			// Include Boolean (true/false) definitions

#include	"CHRPMini.h"			// Include CHRP3 constant symbols and functions

// TODO Initialize oscillator, ports and other PIC/CHRP3 hardware features here.

// Initialize oscillator

void initOsc(void)
{
	OSCTUNE = 0x80;				// 3X PLL ratio mode selected
	OSCCON = 0x70;				// Switch to 16MHz HFINTOSC
	OSCCON2 = 0x10;				// Enable PLL, SOSC, PRI OSC drivers turned off
	ACTCON = 0x90;				// Enable active clock tuning for USB operation
	//while(!PLLRDY);				// Wait for PLL lock
}

// Initialize hardware ports and peripherals. Set starting conditions.

void initPorts(void)
{
    // something here
	LATA = 0b00000000;			// Clear Port A latches before configuring PORTA
	ANSELA = 0b00000000;		// Make all Port A pins digital
	TRISA = 0b00101111;			// Set runLED, IR LEDs as outputs in PORTA

	LATB = 0b00000000;			// Clear Port B latches before configuring PORTB
	ANSELB = 0b00000000;		// Set PORTB for Servo output
	TRISB = 0b00000000;			// Set PORTB ports as outputs
	LATC = 0b00000000;			// Clear Port C latches before configuring PORTC
	ANSELC = 0b00000000;		// Make all Port C pins digital
	TRISC = 0b10110000;			// Set receive and transmit lines for IR
								// demodulator U5 and LED11, servo outputs

	T0CON = 0b10000001;			// Enable TMR0 as 16-bit, internal clock, /2
// Set starting I/O conditions.

	PORTA = 0;					// Turn off all PORTA outputs, turn on run LED
	PORTB = 0;					// Turn off all PORTB LEDs
	PORTC = 0;					// Turn off all PORTC outputs
    
    // Initialize user ports and peripherals.

	//ANSELH = 0b00000000;		// Make all PORTB pins digital I/O

	//OPTION_REG = 0b01010111;	// PORTB pull-ups on, TMR0 internal div 256
	// Enable interrupts, if needed.
}

// Initialize analogue PORTA inputs. Call initANA after initPorts if analogue
// inputs will be used. Enables all CHRP3 analogue channels.

void initANA(void)
{	
	LATA = 0b00000000;			// Clear Port A latches before configuring PORTA
	ANSELA = 0b0011111;		// Make RA0,1,2,3 and 5 analogue inputs
	ADCON0 = 0b00000000;		// Analogue channel AN0, A/D converter
	ADCON1 = 0b00000000;		// VDD positive reference, VSS negative reference
	ADCON2 = 0b00001110;		// 2TAD acquisition time, FOSC/64 conversion clock
	TRISA = 0b00101111;			// Set runLED, IR LEDs as outputs in PORTA
}