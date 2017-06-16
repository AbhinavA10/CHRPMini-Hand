/*==============================================================================
    Project: Hand
    Version: 3.1				Date: May 24, 2017
    Target: CHRPMini			Processor: PIC18F25K50
==============================================================================*/

#include    "xc.h"              // XC compiler general include file
#include    "stdint.h"          // Include integer definitions
#include    "stdbool.h"         // Include Boolean (true/false) definitions
#include    "CHRPMini.h"        // Include CHRPMini constant symbols and functions

// TODO Set linker ROM ranges to 'default,-0-1FFF,-2006-2007,-2016-2017' under "Memory model" pull-down.
// TODO Set linker Code offset to '0x2000' under "Additional options" pull-down.


/*==============================================================================
    Variables
==============================================================================*/
unsigned char arcPos[5] = {0, 0, 0, 0, 0};
int nDelay;
unsigned char cMode, cDelay; //0 is matching glove movements, 1 is commands
bool modeSelect = false, isPressed = false, wasTheButtonLetGo = true;
/* Position for each finger. 0 represents open. 255 means fully closed.
 * THUMB: 0
 * INDEX: 1
 * MIDDLE: 2
 * RING: 3
 * PINKIE: 4
 */

/*==============================================================================
    A/D conversion function. Pass the channel to be converted using one of the
    definitions in CHRP3.h. Returns the most significant 8-bits of the result.
==============================================================================*/
unsigned char adConvert(unsigned char chan) {
    ADON = 1; // Turn A-D converter on
    ADCON0 = (ADCON0 & 0b11000011); // Change the conversion channel by wiping
    ADCON0 = (ADCON0 | chan); // CHS bits and ORing with selected channel

    NOP(); // Wait to allow the input to stabilize
    NOP(); // NOPE
    NOP();
    NOP();

    GO = 1; // Start the conversion

    while (GO); // Wait for the conversion to finish

    ADON = 0; // Turn A-D converter off
    return (ADRESH); // Return the upper 8-bits of the result
}

/*==============================================================================
    Function to convert all analogue flex sensors to 8bit value by 
    calling the A/D conversion function.
==============================================================================*/
void convertSensors() {

    //    arcPos[0] = adConvert(SENSORTHUMB);
    //    arcPos[1] = adConvert(SENSORINDEX);
    //    arcPos[2] = adConvert(SENSORMIDDLE);
    //    arcPos[3] = adConvert(SENSORRING);
    //    arcPos[4] = adConvert(SENSORPINKIE);

}

/*==============================================================================
    Check to see if the mode needs to change
==============================================================================*/
unsigned char checkMode() {
    unsigned char cTempMode = cMode;
    if (S1 == 0) {
        isPressed = true;
        if (!modeSelect && wasTheButtonLetGo) {
            cDelay++;
            if (cDelay == 20) {
                cDelay = 0;
                modeSelect = true;
                wasTheButtonLetGo = false;
                for (unsigned char i = 0; i < 5; i++) {
                    arcPos[i] = 0;
                } //Straighten all fingers
            }
        } else if (wasTheButtonLetGo) {
            cDelay++;
            if (cDelay == 20) {
                cDelay = 0;
                modeSelect = false;
                wasTheButtonLetGo = false;
                cTempMode--;
                for (unsigned char i = 0; i < 5; i++) {
                    arcPos[i] = 0;
                } //Straighten all fingers
            } else {
                for (unsigned char i = 0; i < 5; i++) {
                    arcPos[i] = (cTempMode == i) ? 255 : 0; // Bend the finger that is equal to the mode; Straighten all the others.
                }
            }
        }
    }

    if (S1 == 1) {
        if (isPressed == true && modeSelect) {
            cTempMode++;
            if (cTempMode == 5)cTempMode = 0;
        }
        wasTheButtonLetGo = true;
        cDelay = 0;
        isPressed = false;
    }
    return cTempMode;
}

/*==============================================================================
    Servo function to pulse all 5 servos.
==============================================================================*/
void pulseServos() {

    /*
     * Position is  between 0 and 255
     * Servo code
     * 1-2ms on, 20ms period. 1ms = 90 degrees, 2 ms = -90 degrees
     */

    //Thumb Finger
    SERVOTHUMB = 1; //Turn it on
    __delay_us(540); //540 works well
    for (unsigned char i = arcPos[0]; i != 0; i--) {
        __delay_us(6); //6 for SG90
    }
    SERVOTHUMB = 0; //Turn it off

    //Index Finger
    SERVOINDEX = 1;
    __delay_us(540);
    for (unsigned char i = arcPos[1]; i != 0; i--) {
        __delay_us(6);
    }
    SERVOINDEX = 0;

    //Middle Finger
    SERVOMIDDLE = 1;
    __delay_us(540);
    for (unsigned char i = arcPos[2]; i != 0; i--) {
        __delay_us(6);
    }
    SERVOMIDDLE = 0;

    //Ring Finger
    SERVORING = 1;
    __delay_us(540);
    for (unsigned char i = arcPos[3]; i != 0; i--) {
        __delay_us(6);
    }
    SERVORING = 0;

    //Pinkie Finger
    SERVOPINKIE = 1;
    __delay_us(540);
    for (unsigned char i = arcPos[4]; i != 0; i--) {
        __delay_us(6);
    }
    SERVOPINKIE = 0;
}

/*==============================================================================
 Delay function to delay by the correct amount of time for the time delay
==============================================================================*/
void delay() {
    nDelay = (int) ((20000 - 2500 - 540 * 5 - arcPos[0] - arcPos[1] - arcPos[2] - arcPos[3] - arcPos[4]) / 6); // - 540*5 - arcPos[0] - arcPos[1] - arcPos[2] - arcPos[3] - arcPos[4]);
    for (int i = nDelay; i != 0; i--) {
        __delay_us(6);
    }
}

/*==============================================================================
 Main program code.
==============================================================================*/
int main(void) {
    initOsc(); // Initialize oscillator and wait for it to stabilize
    initPorts(); // Initialize CHRP3 I/O and peripherals
    initANA(); // Initialize Port A analogue inputs
    cMode = 0;
    cDelay = 0;
    while (1) {
        switch (cMode) {
            case 0:
                convertSensors();
                break;
            default:
                break;
        }
        pulseServos();
        //__delay_ms(14); // need to fix this delay
        //__delay_us(4500);
        delay();
        cMode = checkMode();
        if (S1 == 0) // If S1 held, go into mode select. Each time S1 is pressed, switch mode. Confirm mode by holding.
        {

        }
    }
}