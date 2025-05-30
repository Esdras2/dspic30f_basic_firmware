// ========= CONFIG BITS (dSPIC30F4011 + 20 MHz cristal HS) =========
#pragma config FPR     = HS            // Oscilador HS (10-25 MHz, sin PLL)
#pragma config FOS     = PRI           // Fuente = Oscilador primario
#pragma config FCKSMEN = CSW_FSCM_OFF  // Sin clock-switching / sin FSCM

#pragma config PWMPIN  = RST_IOPIN       // PWM sale como GPIO al reset
#pragma config LPOL    = PWMxL_ACT_HI    // Pulso alto = ON
#pragma config HPOL    = PWMxH_ACT_HI    // (para coherencia)

#pragma config WDT     = WDT_OFF       // Watchdog desactivado
#pragma config FPWRT   = PWRT_64       // 64 ms POR
#pragma config BODENV  = BORV20
#pragma config BOREN   = PBOR_ON
#pragma config MCLRE   = MCLR_EN

#pragma config GWRP    = GWRP_OFF
#pragma config GCP     = CODE_PROT_OFF
#pragma config ICS     = ICS_PGD       // Debug/Programación por PGEC/PGED primarios
// ================================================================


#define FCY            5000000UL   // FOSC/4  ? 20 MHz/4 = 5 MHz (importante)
#define PWM_FREQ_HZ    10000UL     // frecuencia PWM deseada (10 kHz)
#define RAMP_PERIOD_MS 2000UL      // ciclo completo subir+bajar (2 s)

#include <xc.h>
#include <libpic30.h>

#define FCY 5000000UL      // 5 MHz
void pwm2l_init(uint16_t duty_ticks)      // duty_ticks = 0..2*PTPER
{
    /* 1. Time-base 10 kHz */
    PTCONbits.PTCKPS = 0;                 // 1:1
    PTCONbits.PTMOD  = 0;                 // free-run
    PTPER = 499;

    /* 2. Canal 2 → RE2 */
    PWMCON1bits.PEN2L = 1;                // Pin bajo al PWM
    PWMCON1bits.PMOD2 = 1;                // Independiente

    /* 3. Ciclo útil inicial */
    PDC2 = duty_ticks;                    // p.ej. 0-1000 para 0-100 %

    /* 4. Sincronía inmediata */
    PWMCON2bits.IUE = 1;

    /* 5. Arranque */
    PTCONbits.PTEN = 1;
}

int main(void)
{
    pwm2l_init(250);      // 50 % duty
    while (1) { /* tu aplicación */ }
}