/*==============================================================================
    Project: Hand
    Version: 3.1				Date: May 29, 2017
    Target: CHRPMini			Processor: PIC18F25K50
==============================================================================*/
#include    "xc.h"              // XC compiler general include file
#include    "stdint.h"          // Include integer definitions
#include    "stdbool.h"         // Include Boolean (true/false) definitions
#include    "CHRPMini.h"        // Include CHRPMini constant symbols and functions

// Have set linker ROM ranges to 'default,-0-1FFF,-2006-2007,-2016-2017' under "Memory model" pull-down.
// Have set linker Code offset to '0x2000' under "Additional options" pull-down.

/*==============================================================================
    Finger Pathing
==============================================================================*/
#define THUMB           0	// Port B value for both motors stopped
#define INDEX           1
#define MIDDLE          2
#define RING            3
#define PINKIE          4

/*==============================================================================
    VARIABLES
==============================================================================*/
unsigned char arcPos[5], arcMaxBendPos[5] = {255, 255, 255, 255, 255}; //Position for each finger. 0 represents open. 255 means fully closed.
unsigned char cMode, cDelay, cGesture, cCountFingerCycle, cCycleIncrement;
//cMode is the mode the hand is in. i.e, decides what the hand will do
//cDelay is for counting duration when mode select button is held 
//cGesture is for the command number
//cCountFingerCycle and cCycleIncrement are used in creating the "come here" gesture
bool modeSelect, isPressedForMode, isPressedForGesture, buttonWasLetGo, calibMode;
// the above variables are needed to properly navigate mode selection, calibration, and gesture cycling
int nDelay, nCalibrationCounter;
//nDelay is for keeping track of the proper delay to pulse servos every 20ms
//nCalibrationCounter is for having the calibration mode on for only 10 seconds/beeps.

/*==============================================================================
    SET POS
        Function to individually set position of each servo to the passed value.
        We are not able to set an entire array to different values, once it is initialized.
        So to avoid recreating the array, or importing <algorithm> this function
        was made to change the position to hard-coded values.
        This will allow us to have certain gestures on command.
==============================================================================*/
void setPos(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e) {
    arcPos[THUMB] = a;
    arcPos[INDEX] = b;
    arcPos[MIDDLE] = c;
    arcPos[RING] = d;
    arcPos[PINKIE] = e;
}

/*==============================================================================
    BEEP
        A generic beep function. Used in other parts of the code so that 
        we can tell what mode it is in.
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
    calibMode = false;
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
                isPressedForMode = false;
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
                isPressedForMode = false;
                beep(400, 500);
                //cTempMode--; // to compensate for the cTempMode++ some lines below
                if (cTempMode == 0) {
                    calibMode = true;
                } else {
                    calibMode = false;
                }
                setPos(0, 0, 0, 0, 0); //Straighten all fingers
            }// else { // user has pressed the button to change the mode

            //}
        }
    }

    if (S1 == 1) { // mode is changed after the button is let go. This is done so that the user is able to leave mode select.
        if (buttonWasLetGo && isPressedForMode && modeSelect) {
            cTempMode++;
            if (cTempMode == 5) {
                cTempMode = 0;
            }
            for (unsigned char i = 0; i < 5; i++) {
                arcPos[i] = (cTempMode == i) ? 255 : 0; // Bend the finger that is equal to the mode; Straighten all the others.
            }
        }
        buttonWasLetGo = true;
        cDelay = 0;
        isPressedForMode = false;
    }
    return cTempMode;
}

/*==============================================================================
    PULSE SERVOS 
        Servo function to pulse all 5 servos one after the other.
==============================================================================*/
void pulseServos() {
    /*  Servo specifications
     * The servo turns 180 degrees. Below values were found by trail and error
     * as the datasheet was an approximation
     * 0.54ms to 2.07ms on time, and a 20ms period.
     */

    //Thumb
    SERVOTHUMB = 1; //Turn it on
    __delay_us(540); //540 works well to give a -90 degree pulse.
    for (unsigned char i = 255 - arcPos[THUMB]; i != 0; i--) {
        /*
         * The 255- arcPos[x] is needed so that the servo turns in the opposite 
         * direction i.e., starts at max position
         * This stops the threads in the hand from crossing over each other.
         */
        __delay_us(6); //For the SG90 servo, a 6us delay for each position
        // allows a max pulse of 2.07ms for +90 degrees.
    }
    SERVOTHUMB = 0; //Turn it off

    //Index Finger
    SERVOINDEX = 1;
    __delay_us(540);
    for (unsigned char i = 255 - arcPos[INDEX]; i != 0; i--) {
        __delay_us(6);
    }
    SERVOINDEX = 0;

    //Middle Finger
    SERVOMIDDLE = 1;
    __delay_us(540);
    for (unsigned char i = 255 - arcPos[MIDDLE]; i != 0; i--) {
        __delay_us(6);
    }
    SERVOMIDDLE = 0;

    //Ring Finger
    SERVORING = 1;
    __delay_us(540);
    for (unsigned char i = 255 - arcPos[RING]; i != 0; i--) {
        __delay_us(6);
    }
    SERVORING = 0;

    //Pinkie Finger
    SERVOPINKIE = 1;
    __delay_us(540);
    for (unsigned char i = 255 - arcPos[PINKIE]; i != 0; i--) {
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
        added as an approximated adjustment factor to account for clock cycles/delays in 
        other parts of the code.
==============================================================================*/
void delay() {
    nDelay = (int) ((20000 - 2500 - 540 * 5 - arcPos[THUMB] - arcPos[INDEX] - arcPos[MIDDLE] - arcPos[RING] - arcPos[PINKIE]) / 6);
    for (int i = nDelay; i != 0; i--) {
        __delay_us(6);
    }
}

/*==============================================================================
    CONVERT SENSORS
        Function to convert all analog flex sensors to 8bit value by 
        calling the A/D conversion function.
        
        The position must equal 255 minus the value that the A/D conversion returns.
        This is because the conductivity of the foam increases as it is bent, 
        meaning less resistance, and therefore a lower value. Without this, the
        hand would start as a fist, and then unbend as the user bends the glove.
==============================================================================*/
void convertSensors() {
    arcPos[THUMB] = 255 - adConvert(SENSORTHUMB);
    arcPos[INDEX] = 255 - adConvert(SENSORINDEX);
    arcPos[MIDDLE] = 255 - adConvert(SENSORMIDDLE);
    arcPos[RING] = 255 - adConvert(SENSORRING);
    arcPos[PINKIE] = 255 - adConvert(SENSORPINKIE);
}

/*==============================================================================
    CENSOR FINGER
        Function to censor out the middle finger. i.e., can't give someone
        the finger.
==============================================================================*/
void censorFinger() {
    if (!calibMode) {
        unsigned char cMin = 255;
        if (arcPos[MIDDLE] < arcPos[THUMB] && arcPos[MIDDLE] < arcPos[INDEX] && arcPos[MIDDLE] < arcPos[RING] && arcPos[MIDDLE] < arcPos[PINKIE]) {
            // to find the size of array in C, you need to use sizeof(). Instead of
            //that, we just hard-coded in 5 in the line below
            for (unsigned char i = 0; i < 5; i++) {
                if (cMin > arcPos[i] && i != MIDDLE) {
                    cMin = arcPos[i];
                }
            }
            arcPos[MIDDLE] = cMin;
        }
    }
}

/*==============================================================================
    CALIBRATION
        Function to convert all analog flex sensors to 8bit value by 
        calling the A/D conversion function.
==============================================================================*/
void calibrate() {
    modeSelect = false; // just to make sure it's not in mode select
    for (unsigned char i = 0; i < 5; i++) {
        if (arcPos[i] < arcMaxBendPos[i] && arcPos[i] > 0) {
            arcMaxBendPos[i] = arcPos[i];
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
                setPos(200, 0, 0, 255, 255); // Peace sign
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
    initPorts(); // Initialize CHRPMini I/O and peripherals
    initANA(); // Initialize Port A analog inputs (for Flex sensors)
    initVariables(); // Initialize all variables 
    while (1) {
        if (!modeSelect) {
            switch (cMode) {
                case 0: // matching glove movements
                    convertSensors();
                    //censorFinger();
                    //if (calibMode) calibrate();
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
        if (!calibMode || modeSelect) {
            pulseServos();
            delay();
        }
        cMode = checkMode();
    }
}