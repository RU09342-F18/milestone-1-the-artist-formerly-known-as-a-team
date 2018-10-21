/* --COPYRIGHT--,BSD_EX
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************
 *
 *                       MSP430 CODE EXAMPLE DISCLAIMER
 *
 * MSP430 code examples are self-contained low-level programs that typically
 * demonstrate a single peripheral function or device feature in a highly
 * concise manner. For this the code may rely on the device's power-on default
 * register values and settings such as the clock configuration and care must
 * be taken when combining code from several examples to avoid potential side
 * effects. Also see www.ti.com/grace for a GUI- and www.ti.com/msp430ware
 * for an API functional library-approach to peripheral configuration.
 *
 * --/COPYRIGHT--*/
//******************************************************************************
//   MSP430G2xx3 Demo - USCI_A0, 9600 UART Echo ISR, DCO SMCLK
//
//   Description: Echo a received character, RX ISR used. Normal mode is LPM0.
//   USCI_A0 RX interrupt triggers TX Echo.
//   Baud rate divider with 1MHz = 1MHz/9600 = ~104.2
//   ACLK = n/a, MCLK = SMCLK = CALxxx_1MHZ = 1MHz
//
//                MSP430G2xx3
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          --|RST          XOUT|-
//            |                 |
//            |     P1.2/UCA0TXD|------------>
//            |                 | 9600 - 8N1
//            |     P1.1/UCA0RXD|<------------
//
//   D. Dang
//   Texas Instruments Inc.
//   February 2011
//   Built with CCS Version 4.2.0 and IAR Embedded Workbench Version: 5.10
//******************************************************************************

/*Nicholas Klein
 *Created 10/10/18   Last Edit: 10/20/18
 *Stranger Things RBG LED milestone 1
*/

#include <msp430.h>

char length;
char count;

void setupLEDs () {
    //RED 1.6
    P1SEL |= BIT6;                           //Sets P1.6 as an I/O pin
    P1SEL2 &= ~BIT6;                          //Sets P1.6 as an I/O pin
    P1DIR |= BIT6;                            //Sets P1.6 as an output
    //Green 2.1
    P2SEL |= BIT1;                           //Sets P2.1 as an I/O pin
    P2SEL2 &= ~BIT1;                          //Sets P2.1 as an I/O pin
    P2DIR |= BIT1;                            //Sets P2.1 as an output
    //Blue 2.4
    P2SEL |= BIT4;                           //Sets P2.4 as an I/O pin
    P2SEL2 &= ~BIT4;                          //Sets P2.4 as an I/O pin
    P2DIR |= BIT4;                            //Sets P2.4 as an output
}

void setupTimers() {
    TA0CTL = TASSEL_2 + ID_2 + MC_1 + TACLR;  //TA0 SM clock, up, div4, clear
    TA1CTL = TASSEL_2 + ID_2 + MC_1 + TACLR;  //TA1 SM clock, up, div4, clear
    TA0CCR0 = 255;                            //TA0 PWM is 1Mhz
    TA1CCR0 = 255;                            //TA1 PWM is 1Mhz
    TA0CCR1 = 0;                              //TA0 duty cycle is 0%
    TA1CCR1 = 0;                              //TA1 duty cycle is 0%
    TA1CCR2 = 0;                              //TA1 duty cycle is 0%
    TA0CCTL1 = OUTMOD_3;                      //Clock is set/reset
    TA1CCTL1 = OUTMOD_3;                      //Clock is set/reset
    TA1CCTL2 = OUTMOD_3;                      //Clock is set/reset
}

void setupUART() {
    DCOCTL = 0;                               // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
    DCOCTL = CALDCO_1MHZ;

    //setup rx and tx
    P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2 ;                    // P1.1 = RXD, P1.2=TXD

    UCA0CTL1 |= UCSSEL_2;                     // SMCLK
    UCA0BR0 = 104;                            // 1MHz 9600
    UCA0BR1 = 0;                              // 1MHz 9600
    UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
    UC0IE |= UCA0RXIE;                        // Enable UART based interrupt
}

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  if (CALBC1_1MHZ==0xFF)                    // If calibration constant erased
  {
    while(1);                               // do not load, trap CPU!!
  }
  setupUART();
  setupTimers();
  setupLEDs();

  //Setup UART LED
  P1DIR |= BIT0;                            //Sets P1.0 as an output
  P1OUT &= ~BIT0;                           //Clears P1.0

  __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0, interrupts enabled
}

//  Echo back RXed character, confirm TX buffer is ready first
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
    P1OUT |= BIT0;              // LED shows if UART is active
    UC0IE |= UCA0TXIE;          // TX interrupt set
    char input = UCA0RXBUF;     // Stores the input

    if (count == 0){
        count = input;
        length = input;
        if (input >= 8) {       // sends the next node the packet length
            UCA0TXBUF = input - 3;
        }
    } else if (length - count == 1){
        TA0CCR1 = input;        // Sets red LED duty cycle
    } else if (length - count == 2){
        TA1CCR1 = input;        // Sets green LED duty cycle
    } else if (length - count == 3){
        TA1CCR2 = input;        // Sets blue LED duty cycle
    } else {
        if (length >= 8) {      //Each unused bit it transmitted
            UCA0TXBUF = input;
        }
    }

    if ((0 - count) <= length) {
        count--;                // Decrements counter to differentiate which bytes to send and which to use
    }
    else {
        count = 0;              // Resets counter after packet is done
    }
    P1OUT &= ~BIT0;             // Turns off onboard LED
}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{
   P1OUT |= BIT0;               //LED shows if UART is active
   UC0IE &= ~UCA0TXIE;          //TX interrupt cleared
   P1OUT &= ~BIT0;              //LED shuts off
}
