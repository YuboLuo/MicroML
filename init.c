/*
 * This file contains functions which initialize the device. This initialization
 * includes MSP pins, UART configuration, and the clock system.
 */
#include "init.h"

void init_gpio(void) {
    /**
     * Initializes all pins to output and sets pins to LOW. This
     * prevents unnecessary current consumption by floating pins.
     */
    P1DIR = 0xFF;
    P1OUT = 0x0;
    P2DIR = 0xFF;
    P2OUT = 0x0;
    P3DIR = 0xFF;
    P3OUT = 0x0;
    P4DIR = 0xFF;
    P4OUT = 0x0;
    P5DIR = 0xFF;
    P5OUT = 0x0;
    P6DIR = 0xFF;
    P6OUT = 0x0;
    P7DIR = 0xFF;
    P7OUT = 0x0;
    P8DIR = 0xFF;
    P8OUT = 0x0;
    PADIR = 0xFF;
    PAOUT = 0x0;
    PBDIR = 0xFF;
    PBOUT = 0x0;
    PCDIR = 0xFF;
    PCOUT = 0x0;
    PDDIR = 0xFF;
    PDOUT = 0x0;

    PM5CTL0 &= ~LOCKLPM5;
}


void init_clock_system(void) {
    /**
     * Initializes the UART system by setting the correct baudrate.
     */
    // Set clock system with DCO of ~1MHz
    CSCTL0_H = CSKEY_H;  // Unlock clock system control registers
    CSCTL1 = DCOFSEL_0;  // Set DCO to 1MHz
    CSCTL2 = SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 =  DIVA__1 | DIVS__1 | DIVM__1;  // Set dividers
    CSCTL0_H = 0;   // Lock the clock system control registers

}

