/***********************************************************************
 *  Project:   Minimal UART2 Demo Firmware
 *  File:      main.c
 *  Device:    dsPIC30F4011 @ 7.37 MHz FRC + PLL×8  (Fosc ≈ 58.96 MHz)
 *             FCY = Fosc / 4 ≈ 14.74 MHz
 *  Author:    Esdras Vázquez León
 *  Created:   3 Jul 2025
 *
 *  Overview
 *  --------
 *  ▸ System clock: Internal FRC oscillator with PLL×8 (no external crystal).
 *  ▸ UART2: 115 200 baud, 8 data bits, no parity, 1 stop bit (8‑N‑1).
 *  ▸ Main loop: Transmits "UART ready" banner once, then echoes every
 *               received byte back to the terminal.
 *
 *  License: MIT — adapt as needed.
 ***********************************************************************/

/*======================================================================*/
/*  CONFIGURATION BITS (XC16 v2)                                        */
/*======================================================================*/
#pragma config FPR     = FRC_PLL8        // Primary oscillator = FRC×8
#pragma config FOS     = PRI             // Use primary after reset
#pragma config FCKSMEN = CSW_FSCM_OFF    // No clock switching / fail‑safe

#pragma config PWMPIN  = RST_PWMPIN      // PWM pins are GPIO on reset
#pragma config LPOL    = PWMxL_ACT_HI
#pragma config HPOL    = PWMxH_ACT_HI

#pragma config WDT     = WDT_OFF         // Watchdog Timer OFF
#pragma config FPWRT   = PWRT_64         // Power‑up Timer 64 ms
#pragma config BODENV  = BORV42          // Brown‑out ≈ 4.2 V
#pragma config BOREN   = PBOR_OFF
#pragma config MCLRE   = MCLR_EN         // /MCLR enabled
#pragma config GWRP    = GWRP_OFF        // Code write protect OFF
#pragma config GCP     = CODE_PROT_OFF   // Code read protect OFF
#pragma config ICS     = ICS_PGD         // ICD on PGEC/PGED

/*======================================================================*/
/*  FREQUENCY DEFINES                                                   */
/*======================================================================*/
#define _CRYSTAL_WORK   (7370000UL * 8UL)   // 7.37 MHz ×8 → ≈ 58.96 MHz
#define FCY             (_CRYSTAL_WORK/4UL) // Instruction clock ≈ 14.74 MHz

/*======================================================================*/
/*  INCLUDES                                                            */
/*======================================================================*/
#include <xc.h>
#include <stdint.h>
#include <libpic30.h>         // __delay_ms / __delay_us macros

/*======================================================================*/
/*  UART CONSTANTS & MACROS                                             */
/*======================================================================*/
#define BAUD_RATE  115200UL
/* BRG = (FCY / (16 * BAUD)) – 1 ; BRGH = 0 */
#define BRGVAL     ((FCY / (16UL * BAUD_RATE)) - 1UL)  // ≈ 7 @ 14.74 MHz

/* RP pin mapping for UART2 */
#define U2TX_RP  21   // RF5 → RP21
#define U2RX_RP  20   // RF4 → RP20

/*======================================================================*/
/*  UART FUNCTIONS                                                      */
/*======================================================================*/
static void uart2_init(void)
{
    /* Configure pins */
    TRISFbits.TRISF4 = 1;      // RX input
    TRISFbits.TRISF5 = 0;      // TX output

    /* PPS mapping (dsPIC30F: fixed ‑ only ensure pins are correct) */

    U2MODE = 0;                // Disable while configuring
    U2MODEbits.PDSEL = 0b00;   // 8‑bit, no parity
    U2MODEbits.STSEL = 0;      // 1 stop bit

    U2BRG = (uint16_t)BRGVAL;  // Set baud rate

    U2STA = 0;
    U2MODEbits.UARTEN = 1;     // Enable UART2
    __delay_us(50);            // Wait ≥ 1‑2 instruction cycles
    U2STAbits.UTXEN = 1;       // Enable transmitter

    /* Clear interrupt flags (not used in polling demo) */
    IFS1bits.U2RXIF = 0;
    IFS1bits.U2TXIF = 0;
}

static inline void uart2_putc(char c)
{
    while (U2STAbits.UTXBF);   // Wait if TX FIFO full
    U2TXREG = (uint8_t)c;
}

static void uart2_puts(const char *s)
{
    while (*s) uart2_putc(*s++);
}

/*======================================================================*/
/*  MAIN                                                                */
/*======================================================================*/
int main(void)
{
    /* Initialise clock (already configured by config bits) & UART */
    uart2_init();

    /* Send banner once */
    uart2_puts("\r\n=== UART ready (115200 8N1) ===\r\n");

    /* Polling echo loop */
    while (1) {
        if (U2STAbits.URXDA) {        // Data available?
            char rx = U2RXREG;        // Read
            uart2_putc(rx);           // Echo back
        }
    }

    return 0;   // never reached
}
