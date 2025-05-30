// ------------- CONFIG BITS (20 MHz HS, sin PLL) -------------
#pragma config FPR     = HS
#pragma config FOS     = PRI
#pragma config FCKSMEN = CSW_FSCM_OFF
#pragma config PWMPIN  = RST_IOPIN
#pragma config LPOL    = PWMxL_ACT_HI
#pragma config HPOL    = PWMxH_ACT_HI
#pragma config WDT     = WDT_OFF
#pragma config FPWRT   = PWRT_64
#pragma config BODENV  = BORV20
#pragma config BOREN   = PBOR_ON
#pragma config MCLRE   = MCLR_EN
#pragma config GWRP    = GWRP_OFF
#pragma config GCP     = CODE_PROT_OFF
#pragma config ICS     = ICS_PGD
// -------------------------------------------------------------

#include <xc.h>
#include <libpic30.h>

/* ======== Constantes ======== */
#define FCY             5000000UL   // 20 MHz / 4
#define PWM_FREQ_HZ     10000UL     // 10 kHz PWM
#define RAMP_PERIOD_MS  2000UL      // ciclo 2 s
#define STEP_MS         10UL        // resolución 10 ms

/* === LED blink en RD1 === */
#define BLINK_TRIS  TRISDbits.TRISD1
#define BLINK_LAT   LATDbits.LATD1

/* === Variables para PWM2L (RE2) === */
static volatile uint16_t duty       = 0;
static volatile uint16_t duty_max   = 0;
static volatile uint16_t duty_step  = 0;
static volatile int8_t   dir        = 1;
/* ---------- PWM1L / RE0 ---------- */
/*  Libera RE0 y lo entrega al PWM 1L  */
static void unlock_RE0_for_PWM(void)
{
    /* 1 . — Apaga A/D en ese pin (AN5) y el módulo ADC   */
    ADPCFGbits.PCFG5 = 1;      // AN5 → digital
    ADCON1bits.ADON  = 0;      // ADC global OFF

    /* 2 . — Deshabilita comparadores analógicos          */
    CMCON = 0;                 // ambas salidas comparador desactivadas

    /* 3 . — Evita INT0 sobre ese pin                     */
    IEC0bits.INT0IE = 0;       // INT0 interrupt disable
    IFS0bits.INT0IF = 0;       // clear flag (por higiene)

    /* 4 . — Apaga QEI1 e Input-Capture 1                 */
    QEICONbits.QEIM  = 0;      // QEI1 OFF
    IC1CONbits.ICM   = 0;      // IC1 OFF

    /* 5 . — Asegura que no haya override manual          */
    OVDCONbits.POVD1L = 0;     // PWM módulo controla el pin

    /* 6 . — Pone el pin en salida y lo asigna al PWM     */
    TRISEbits.TRISE0  = 0;     // salida digital
    PWMCON1bits.PMOD1 = 1;     // canal 1 independiente
    PWMCON1bits.PEN1L = 1;     // RE0 (PWM1L) bajo control PWM
}


static void initPWM1L(void)
{
    TRISEbits.TRISE0 = 0;           // RE0 como salida

    PTCONbits.PTEN = 0;
    PTCONbits.PTCKPS = 0;
    PTCONbits.PTMOD  = 0;
    PTPER = (FCY / PWM_FREQ_HZ) - 1;

    PWMCON1bits.PMOD1 = 1;          // modo independiente
    PWMCON1bits.PEN1L = 1;          // RE0 controlado por PWM1L
    DTCON1  = 0;
    OVDCON  = 0;

    duty_max  = (PTPER << 1);       // 100 %
    duty_step = duty_max / (RAMP_PERIOD_MS / (2 * STEP_MS));
    duty      = 0;
    PDC1      = duty;               // *** usa PDC1 ***
    OVDCONbits.POVD1L = 1;

    PWMCON2bits.IUE = 1;
    PTCONbits.PTEN  = 1;            // arranca PWM
}

/* ========== Timer1: actualiza duty cada 10 ms ========== */
#define T1_PRESC 64
#define T1_PR    ((FCY / T1_PRESC / 1000) * STEP_MS - 1) // ~780

static void initTimer1(void)
{
    T1CON = 0;
    PR1   = T1_PR;
    T1CONbits.TCKPS = 0b10;          // 1:64
    IFS0bits.T1IF   = 0;
    IEC0bits.T1IE   = 1;
    IPC0bits.T1IP   = 4;
    T1CONbits.TON   = 1;
}

/* ---- en la ISR de Timer-1 ---- */
void __attribute__((interrupt, auto_psv)) _T1Interrupt(void)
{
    IFS0bits.T1IF = 0;
    duty += dir * duty_step;
    if (duty >= duty_max) { duty = duty_max; dir = -1; }
    else if (duty == 0)   { dir = +1; }
    PDC1 = duty;                          // *** actualiza PWM1L/RE0 ***
}

/* ======== Timer2: blink LED cada 100 ms ======== */
#define BLINK_MS 100UL
#define T2_PRESC 64
#define T2_PR    ((FCY / T2_PRESC / 1000) * BLINK_MS - 1) // ~7812

static void initTimer2(void)
{
    // LED en RD1
    BLINK_TRIS = 0;
    BLINK_LAT  = 0;

    T2CON = 0;
    PR2   = T2_PR;
    T2CONbits.TCKPS = 0b10;          // 1:64
    IFS0bits.T2IF = 0;
    IEC0bits.T2IE = 1;
    IPC1bits.T2IP = 3;
    T2CONbits.TON = 1;
}

void __attribute__((interrupt, auto_psv)) _T2Interrupt(void)
{
    IFS0bits.T2IF = 0;
    BLINK_LAT ^= 1;
}

/* ===================== MAIN ===================== */
int main(void)
{
    __builtin_disable_interrupts();
    initPWM1L();
    initTimer1();
    initTimer2();
    __builtin_enable_interrupts();

    while (1) { /* todo por ISRs */ }
    return 0;
}
