/*==============================================================================
    CHRP 3.1 (PIC18F25K50) symbolic constant definitions.
==============================================================================*/

// TODO - Add other user constant definitions for CHRP3 hardware here.

// Inputs read the Port registers (eg. RC0). Outputs write to the port latches
// (eg. LATC0). Create different symbolic names for pins that will be both
// inputs and outputs using the appropriate PORT and LAT pin names.

// PORTA I/O device definitions

#define	Q1			PORTAbits.RA0		// Phototransistor Q1 input  - 
#define	Q2			PORTAbits.RA1		// Phototransistor Q2 input  - 
#define	VR1			PORTAbits.RA2		// Potentiometer VR1 input   - 
#define	T1			PORTAbits.RA3		// Temperature sensor T1 input
#define	LED2		LATAbits.LATA4		// Run LED, LED2 output
#define	RUNLED      LATAbits.LATA4		// Run LED, LED2 output
#define	VM			PORTAbits.RA5		// Motor supply voltage divider input
#define	BEEPER		LATAbits.LATA6		// Piezo beeper P1 output
#define	P1			LATAbits.LATA6		// Piezo beeper P1 output
#define	LED12		LATAbits.LATA7		// Floor/phototransistor LED (LED12) output

// PORTB I/O device definitions

//#define LED10		LATBbits.LATB0		// LED10 output (LSB)
#define M1A			LATBbits.LATB0		// Motor 1 A output
#define M1B			LATBbits.LATB1		// Motor 1 B output
#define M2A			LATBbits.LATB2		// Motor 2 A output
#define M2B			LATBbits.LATB3		// Motor 2 B output

#define H18         PORTBbits.RB7       // External I/O header H18 input 
#define SERVOTHUMB  LATBbits.LATB7      // Servo 1 output (external header H18)
#define H17         PORTBbits.RB6       // External I/O header H17 input 
#define SERVOINDEX  LATBbits.LATB6      // Servo 1 output (external header H17)
#define H16         PORTBbits.RB5       // External I/O header H16 input 
#define SERVOMIDDLE LATBbits.LATB5      // Servo 1 output (external header H16)
#define H15         PORTBbits.RB4       // External I/O header H15 input 
#define SERVORING   LATBbits.LATB4      // Servo 1 output (external header H15)
#define H13         PORTBbits.RB2       // External I/O header H13 input 
#define SERVOPINKIE LATBbits.LATB2      // Servo 1 output (external header H13)


// PORTC I/O device definitions

#define LED11		LATCbits.LATC6		// Communication/sensor LED, LED11 output
#define IR			PORTCbits.RC7		// IR demodulator U5 input
#define U5			PORTCbits.RC7		// IR demodulator U5 input

// PORTE input pin definition

#define	S1			PORTEbits.RE3		// Switch S1 input

// Analogue-digital converter input channel definitions

#define SENSORTHUMB     0b00000000		// Flex Sensor A-D input channel (Ch0) on AN0/RA0
#define SENSORINDEX     0b00000100		// Flex Sensor A-D input channel (Ch1) on AN1/RA1
#define SENSORMIDDLE    0b00001000		// Flex Sensor A-D input channel (Ch2) on AN2/RA2
#define SENSORRING      0b00001100		// Flex Sensor A-D input channel (Ch3) on AN3/RA3
#define SENSORPINKIE    0b00010000		// Flex Sensor A-D input channel (Ch4) on AN4/RA5

#define	ADVM		0b00010000		// Motor voltage divider A-D input channel (Ch4)
#define ADTD		0b01110000		// PICmicro on-die temperature diode

// Clock frequency for delay macros and simulation

#define _XTAL_FREQ	48000000		// Processor clock frequency for time delays

// TODO - Add function prototypes for all functions in CHRP3.c here.

void initOsc(void); // Oscillator initialization function prototype.
void initPorts(void); // Port initialization function prototype.
void initANA(void); // Analogue PORTA initialization function.