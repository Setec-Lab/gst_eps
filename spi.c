/* 
 * File: spi.c
 * Author: Adrian Wenzel
 * Author: Juan J. Rojas
 * Created on 4th December 2019
 */

// Includes
#include "spi.h"
#include <xc.h>
#include <stdio.h>
#include <stdlib.h>

// Macros and defines
#ifndef _XTAL_FREQ
    #define _XTAL_FREQ 32000000
#endif

void init(void) {
    OSCCONbits.SCS = 0x02; // Set clock source to internal oscillator
    OSCCONbits.IRCF = 0x0F; // Set clock to 16 MHz
    LATB = 0x00; // Port B: Output Latch Low
    TRISB &= ~0x10; // Set RB4 as output
    for (uint8_t i = 0; i < 5; i++) {
        RB4 = 1; 
        delay_ms(200);
        RB4 = 0;
        delay_ms(200);
    }
}


void spi_slave_init(uint8_t cpol, uint8_t cpha, bool SS_active) {
    // SSPSTAT - register configuration
    SSPSTATbits.SMP = 0;        // SMP bit must be cleared when in slave mode
    if (cpha)                   // CPHA = !CKE
        SSPSTATbits.CKE = 0;    // CPHA = 1: read data on 2nd edge of CLK line (active -> idle transition)
    else
        SSPSTATbits.CKE = 1;    // CPHA = 0: read data on 1st edge of CLK line (idle -> active transition)
    
    // SSPCON1 - register configuration
    if (cpol)                   // CPOL = CKP
        SSP1CON1bits.CKP = 1;   // Clock idle state is a HIGH level
    else
        SSP1CON1bits.CKP = 0;   // Clock idle state is a LOW level
    if (SS_active) {              // slave select input active
        SSP1CON1bits.SSPM = 0x04; // (= 0b0100) SPI slave mode, !SS pin control enabled
        ANSA5 = 0;                // = Digital I/O: Pin is assigned to port or digital special function
        TRISA5 = 1;               // set pin A5 as input: !SS
    } else {                      // don't use slave select input 
        SSP1CON1bits.SSPM = 0x05; // (= 0b0101) SPI slave mode, !SS pin control disabled
    }
    // set further input and output pins accordingly
    TRISC3 = 1; // input:  SCK
    TRISC4 = 1; // input:  SDI (MOSI)
    TRISC5 = 0; // output: SDO (MISO)
    
    /* Before enabling SPI module in slave mode, clock line (CLK) must match the
       proper idle state. Clock line can be observed by reading SCK pin. Idle
       state is determined by CKP bit of SSPCON1 register. */
    if (cpol) {         // Clock idle state is a HIGH level
        while (!RC3);   // wait for SCK to go to Idle state (=LOW on PIN RC3)
    } else {            // Clock idle state is a LOW level
        while (RC3);    // wait for SCK to go to Idle state (=HIGH on PIN RC3)
    }
    SSPCON1bits.SSPEN = 1; // enable SPI module
}
