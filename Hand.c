/*==============================================================================
    Project: Hand
    Version: 3.1				Date: May 24, 2017
    Target: CHRPMini			Processor: PIC18F25K50
==============================================================================*/
//need to make convert sensors jitter less. can do this by having steps instead of exact values\
// censoring stuff in convert sensors does not work.
#include    "xc.h"              // XC compiler general include file
#include    "stdint.h"          // Include integer definitions
#include    "stdbool.h"         // Include Boolean (true/false) definitions
#include    "CHRPMini.h"        // Include CHRPMini constant symbols and functions

// Have set linker ROM ranges to 'default,-0-1FFF,-2006-2007,-2016-2017' under "Memory model" pull-down.
// Have set linker Code offset to '0x2000' under "Additional options" pull-down.

/*==============================================================================
    Finger Pathing
==============================================================================*/
/*
 * THUMB:   0
 * INDEX:   1
 * MIDDLE:  2
 * RING:    3
 * PINKIE:  4
 */

/*==============================================================================
    VARIABLES
==============================================================================*/
unsigned char arcPos[5], arcMinPos[5] = {255, 255, 255, 255, 255}; //Position for each finger. 0 represents open. 255 means fully closed.
unsigned char cMode, cDelay, cCountFingerCycle, cCycleIncrement; //cDelay is for counting duration when mode select button is held 
unsigned char cGesture; // variable for command number
bool modeSelect, isPressedForMode, isPressedForGesture, buttonWasLetGo, calibMode; // variables needed to properly navigate mode selection
int nDelay, nCalibrationCounter; //  for keeping track of the proper delay to pulse servos every 20ms

/*==============================================================================
    SET POS
        Function to individually set position of each servo to the passed value.
        We are not able to set an entire array to different values, once it is initialized.
        So to avoid recreating the array, or importing <algorithm> this function
        was made to change the position to hard-coded values.
        This will allow us to have certain gestures on command.
==============================================================================*/
void setPos(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e) {
    arcPos[0] = a;
    arcPos[1] = b;
    arcPos[2] = c;
    arcPos[3] = d;
    arcPos[4] = e;
}

/*==============================================================================
    BEEP
        Boop.
==============================================================================*/
void beep(unsigned int pitch, unsigned int duration) {
    for (duration; duration != 0; duration--) {
        BEEPER = !BEEPER;
        for (unsigned int j = pitch; j != 0; j--);
    }
}

/*==============================================================================
    INIT VARIABLES
        A function to set starting values for each variable.
        Created so that setting starting values is made simple as it is all in 
        one spot and not when they are created
==============================================================================*/
void initVariables() {
    setPos(0, 0, 0, 0, 0); // starting position of servos is an open hand.
    modeSelect = false;
    isPressedForMode = false;
    isPressedForGesture = false;
    buttonWasLetGo = true;
    calibMode = true;
    cMode = 0;
    cDelay = 0;
    cCountFingerCycle = 5;
    cCycleIncrement = 5;
    nCalibrationCounter = 0;
}

/*==============================================================================
    AD CONVERT
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
    CHECK MODE 
        Function to check if the mode needs to change
        If S1 is held, go into mode select. Each time S1 is pressed, switch mode.
        Confirm mode, and leave mode select by holding S1 again.
==============================================================================*/
unsigned char checkMode() {
    unsigned char cTempMode = cMode;
    if (S1 == 0) {
        isPressedForMode = true;
        if (!modeSelect && buttonWasLetGo) { // user is holding button to enter mode selection
            cDelay++;
            if (cDelay == 20) {
                cDelay = 0;
                modeSelect = true;
                buttonWasLetGo = false;
                beep(200, 500);
                for (unsigned char i = 0; i < 5; i++) {
                    arcPos[i] = (cTempMode == i) ? 255 : 0; // Bend the finger that is equal to the mode; Straighten all the others.
                }
            }
        } else if (buttonWasLetGo) {
            cDelay++;
            if (cDelay == 20) { // user is holding to leave mode select
                cDelay = 0;
                modeSelect = false;
                buttonWasLetGo = false;
                beep(400, 500);
                cTempMode--;
                // to compensate for the cTempMode++ below
                if (cTempMode == 0) {
                    calibMode = true;
                } else {
                    calibMode = false;
                }
                setPos(0, 0, 0, 0, 0); //Straighten all fingers
            } else { // user has pressed the button to change the mode
                for (unsigned char i = 0; i < 5; i++) {
                    arcPos[i] = (cTempMode == i) ? 255 : 0; // Bend the finger that is equal to the mode; Straighten all the others.
                }
            }
        }
    }

    if (S1 == 1) { // mode is changed after the button is let go. This is done so that the user is able to leave mode select.
        if (isPressedForMode && modeSelect) {
            cTempMode++;
            if (cTempMode == 5)cTempMode = 0;
        }
        buttonWasLetGo = true;
        cDelay = 0;
        isPressedForMode = false;
    }
    return cTempMode;
}

/*==============================================================================
    PULSE SERVOS 
        Servo function to pulse all 5 servos.
==============================================================================*/
void pulseServos() {
    /*  Servo specifications
     * The servo turns 180 degrees. Below values were found by trail and error
     * as the datasheet was an approximation
     * 0.54ms to 2.07ms on. 20ms period.
     */

    //Thumb Finger
    SERVOTHUMB = 1; //Turn it on
    __delay_us(540); //540 works well to give a -90 degree pulse.
    for (unsigned char i = arcPos[0]; i != 0; i--) {
        __delay_us(6); //For the SG90 servo, a 6us delay for each position
        // allows a max pulse of 2.07ms for +90 degrees.
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
    DELAY
        Delay function to delay by the correct amount of time such that pulses between
        each servo is always 20ms.
        Takes the goal of a 20ms pulse, subtracts guaranteed servo pulses of 0.54ms
        and then also subtracts any extra pulses for each position. (-2500) was 
        added as an adjustment factor to account for clock cycles in other code.
==============================================================================*/
void delay() {
    nDelay = (int) ((20000 - 2500 - 540 * 5 - arcPos[0] - arcPos[1] - arcPos[2] - arcPos[3] - arcPos[4]) / 6);
    for (int i = nDelay; i != 0; i--) {
        __delay_us(6);
    }
}

/*==============================================================================
    CONVERT SENSORS
        Function to convert all analogue flex sensors to 8bit value by 
        calling the A/D conversion function.
==============================================================================*/
void convertSensors() {
    arcPos[0] = adConvert(SENSORTHUMB);
    arcPos[1] = adConvert(SENSORINDEX);
    arcPos[2] = adConvert(SENSORMIDDLE);
    arcPos[3] = adConvert(SENSORRING);
    arcPos[4] = adConvert(SENSORPINKIE);
    // the lines below censor out the middle finger. i.e., can't give someone
    //    // the finger
    if (!calibMode) {
        unsigned char cMax = 0;
        if (arcPos[2] > arcPos[0] && arcPos[2] > arcPos[1] && arcPos[2] > arcPos[3] && arcPos[2] > arcPos[4]) {
            // to find the size of array in C, you need to use sizeof(). Instead of
            //that, we just hard-coded in 5 in the line below
            for (unsigned char i = 0; i < 5; i++) {
                if (arcPos[i] > cMax && i != 2) {
                    arcPos[i] = cMax;
                }
            }
            arcPos[2] = cMax;
        }
    }
}

/*==============================================================================
    CALIBRATION
        Function to convert all analogue flex sensors to 8bit value by 
        calling the A/D conversion function.
==============================================================================*/
void calibrate() {
    modeSelect = false;
    for (unsigned char i = 0; i < 5; i++) {
        if (arcPos[i] < arcMinPos[i] && arcPos[i] > 0) {
            arcMinPos[i] = arcPos[i];
        }
    }
    nCalibrationCounter++;
    __delay_ms(1);
    if (nCalibrationCounter % 1000 == 0 && nCalibrationCounter != 10000) {
        beep(200, 500);
    }
    if (nCalibrationCounter == 10000) {
        nCalibrationCounter = 0;
        calibMode = false;
        beep(400, 2000);
    }
}

/*==============================================================================
    COMMANDS
        Function to switch between preprogrammed finger positions
        using the button.
==============================================================================*/
void commands() {
    /*
     * We are able to still change the gesture using the same button
     * because entering mode select requires the user to hold the button 
     * for approximately 3 seconds.
     */
    if (S1 == 0 && !modeSelect) {
        isPressedForGesture = true;
    }
    if (S1 == 1 && isPressedForGesture && !modeSelect) { // gesture changes when button is let go
        isPressedForGesture = false;
        cGesture++;
        if (cGesture == 9)cGesture = 0;
        switch (cGesture) {
            case 0:
                setPos(0, 0, 0, 0, 0); // Open hand
                break;
            case 1:
                setPos(255, 255, 255, 255, 255); // Fist
                break;
            case 2:
                setPos(0, 0, 255, 255, 0); // Spider-man
                break;
            case 3:
                setPos(0, 255, 255, 255, 0); // Hang Loose
                break;
            case 4:
                setPos(200, 0, 0, 255, 255); // Peace sign ?
                break;
            case 5:
                setPos(200, 200, 0, 0, 0); // A-OK
                break;
            case 6:
                setPos(0, 255, 255, 255, 255); // Thumbs Up
                break;
            case 7:
                setPos(255, 0, 255, 255, 255); // Index pointing
                break;
            case 8:
                setPos(255, 0, 255, 255, 0); // Rock On
                break;
            default:
                break;
        }
    }
}

/*==============================================================================
    HEY KID, WANT SOME CANDY?
        Function to lure small children towards the van.
        (SPOILER: Kid doesn't actually get candy. We can't afford that)
 
        Update: This function actually cycles the index finger back and forth.
        This creates the "come here" gesture with your finger. Maybe the kid
        really will get candy.
==============================================================================*/
void heyKidWantSomeCandy() {
    setPos(255, cCountFingerCycle, 255, 255, 255);
    cCountFingerCycle += cCycleIncrement;
    if (cCountFingerCycle == 255 || cCountFingerCycle == 0) {
        cCycleIncrement *= -1;
    }
}

/*==============================================================================
 MAIN PROGRAM CODE.
==============================================================================*/
int main(void) {
    initOsc(); // Initialize oscillator and wait for it to stabilize
    initPorts(); // Initialize CHRP3 I/O and peripherals
    initANA(); // Initialize Port A analogue inputs
    initVariables(); // Initialize all variables 
    while (1) {
        if (!modeSelect) {
            switch (cMode) {
                case 0: // matching glove movements
                    //                    convertSensors();
                    //                    if (calibMode) {
                    //                        calibrate();
                    //                    }
                    setPos(0, 0, 0, 0, 0);
                    break;
                case 1: // pre-programmed gestures
                    commands();
                    break;
                case 2:
                    heyKidWantSomeCandy(); // "come here" gesture
                    break;
                default:
                    break;
            }
        }
        if (!calibMode || modeSelect || cMode != 0) {
            pulseServos();
            delay();
        }
        cMode = checkMode();
    }
}