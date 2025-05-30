/* 
 * File:   main.c
 * Author: Esdra
 *
 * Created on 28 de mayo de 2025, 03:25 PM
 */
// ================= OSCILLATOR =================
//========== CONFIG BITS (dsPIC30F4011 · crystal 20 MHz) ==========
#pragma config FPR     = XT        // Primary osc. + PLL×4 (20 MHz ×4 = 80 MHz FOSC)
#pragma config FOS     = PRI            // Fuente = Oscilador primario
#pragma config FCKSMEN = CSW_FSCM_OFF   // Sin clock switching / fail-safe

// ================ WATCHDOG ====================
#pragma config FWPSB   = WDTPSB_16      // 1:16  
#pragma config FWPSA   = WDTPSA_1       // 1:1   |- prescaler ~1 s
#pragma config WDT     = WDT_OFF        // Desactivado durante desarrollo

// ============ POWER-ON / BROWN-OUT ============
#pragma config FPWRT   = PWRT_64        // 64 ms
#pragma config BODENV  = BORV42         // 2,0 V  (cámbialo si lo prefieres a 2,7 V o 4,2 V)
#pragma config BOREN   = PBOR_ON
#pragma config LPOL    = PWMxL_ACT_HI
#pragma config HPOL    = PWMxH_ACT_HI
#pragma config PWMPIN  = RST_IOPIN
#pragma config MCLRE   = MCLR_EN

// ================= SECURITY ===================
#pragma config GWRP    = GWRP_OFF
#pragma config GCP     = CODE_PROT_OFF

// ========== DEBUG / PROGRAMMING ===============
#pragma config ICS     = ICS_PGD        // PGC/PGD primarios
        
#include <stdio.h>
#include <stdlib.h>

// Frecuencia de ciclo de instrucción con FRC y PLL (opcional)

#define FCY 5000000UL              // 5 MIPS
/* ======== Timer-1 → interrupción cada 100 ms ======== */
#define T1_PERIOD 7812             // (FCY/64/10)-1  → 100 ms
#include <libpic30.h>
#include <xc.h>


/* ======== Pin LED (RD0) ======== */
#define LED_TRIS TRISDbits.TRISD0
#define LED_LAT  LATDbits.LATD0


void initTimer1(void)
{
    T1CON = 0;                   // limpia control
    TMR1  = 0;                   // contador a cero
    PR1   = T1_PERIOD;           // periodo 100 ms
    T1CONbits.TCKPS = 2;         // prescaler 1:64
    T1CONbits.TCS   = 0;         // reloj interno (FCY)

    IFS0bits.T1IF = 0;           // limpia bandera
    IEC0bits.T1IE = 1;           // habilita IRQ
    IPC0bits.T1IP = 4;           // prioridad media

    T1CONbits.TON = 1;           // arranca Timer-1
}

/* ISR Timer-1: conmuta LED */
void __attribute__((interrupt, auto_psv)) _T1Interrupt(void)
{
    IFS0bits.T1IF = 0;           // limpia bandera
    LED_LAT ^= 1;                // toggle LED
}

void initLED(void)
{
    LED_TRIS = 0;                // RD0 salida
    LED_LAT  = 0;                // LED apagado
}

int main(void)
{
    __builtin_disable_interrupts();

    initLED();
    initTimer1();

    __builtin_enable_interrupts();

    while (1) { /* lazo vacío, todo por interrupción */ }
    return 0;
}