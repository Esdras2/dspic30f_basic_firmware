/********************  CONFIGURATION BITS  ************************/
#pragma config FPR     = HS                 // HS cristal 20 MHz
#pragma config FOS     = PRI
#pragma config FCKSMEN = CSW_FSCM_OFF       // Sin cambio de reloj / sin monitor
#pragma config PWMPIN  = RST_IOPIN          // PWM pins como IO después de reset
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
/******************************************************************/


/* =======  RELOJ Y CONSTANTES GENERALES  ======= */
#define _XTAL_FREQ   20000000UL           // cristal 20 MHz
#define FCY          (_XTAL_FREQ/4UL)     // 5 MHz
#include <xc.h>
#include <libpic30.h>

/* =======  PWM 10 kHz – 70 %  ======= */
#define PWM_HZ       10000UL              // 10 kHz
#define DUTY_PC      70                   // %
/* Time-base del PWM avanza a FCY/2  →  PTPER = FCY/(2·fPWM) − 1 */
#define PTPER_VAL    ((FCY/(PWM_HZ*2UL)) - 1)          // 249
/* PDCx = (2·(PTPER+1)) · Duty% / 100   (0…2·PTPER+2)            */
#define PDC_70       (((PTPER_VAL + 1UL) * 2UL * DUTY_PC) / 100UL)

/* =======  BLINK 10 Hz en RD1  (Timer 1)  ======= */
#define BLINK_HZ     10UL                 // 10 Hz → 100 ms
#define T1_PRESC     64                   // 1:64  (TCKPS = 0b10)
#define T1_PR        ((FCY / (T1_PRESC * BLINK_HZ)) - 1) // 7811

/* =======  I/O ======= */
#define BLINK_TRIS   TRISDbits.TRISD1
#define BLINK_LAT    LATDbits.LATD1

/* ----------  Timer 1: parpadeo  ---------- */
void __attribute__((interrupt, auto_psv)) _T1Interrupt(void)
{
    IFS0bits.T1IF = 0;            // limpia flag
    BLINK_LAT ^= 1;               // toggle LED
}
static void initBlink_T1(void)
{
    BLINK_TRIS = 0;               // RD1 salida
    BLINK_LAT  = 0;

    T1CON = 0;                    // detiene y limpia
    TMR1  = 0;
    PR1   = T1_PR;                // 100 ms
    T1CONbits.TCKPS = 0b10;       // 1:64
    IPC0bits.T1IP   = 5;          // prioridad alta
    IFS0bits.T1IF   = 0;
    IEC0bits.T1IE   = 1;
    T1CONbits.TON   = 1;          // arranca
}

/* ----------  “Desbloqueo” de pines RE0/2/4  ---------- */
static void unlockPWMpins(void)
{
    ADPCFG = 0xFFFF;      // todos digitales
    ADCON1 = 0;
//    CMCON  = 0;
    IEC0bits.INT0IE = 0;  IFS0bits.INT0IF = 0;
    QEICONbits.QEIM = 0;
    IC1CONbits.ICM  = 0;
    OVDCON = 0;
}

/* ----------  PWM1/2/3 independientes al 70 %  ---------- */
static void initAllPWMs70(void)
{
    unlockPWMpins();

    /* 1) TRIS → salidas (RE0, RE2, RE4) */
    TRISE &= ~(1<<0 | 1<<2 | 1<<4);

    /* 2) PWM off mientras configuras */
    PTCONbits.PTEN   = 0;

    /* 3) Configura base de tiempo PWM */
    PTCONbits.PTCKPS = 0;             // prescaler 1:1
    PTCONbits.PTMOD  = 0;             // free-running mode
    PTPER = PTPER_VAL;                // frecuencia 10 kHz (valor 249)

    /* 4) Canales independientes Low-side activos */
    PWMCON1 = 0;  
    PWMCON1bits.PMOD1 = 1; PWMCON1bits.PEN1L = 1;
    PWMCON1bits.PMOD2 = 1; PWMCON1bits.PEN2L = 1;
    PWMCON1bits.PMOD3 = 1; PWMCON1bits.PEN3L = 1;
    
    /* 5) Sin dead-time */
    DTCON1 = 0;

    /* 6) Duty cycle fijo 70% */
    PDC1 = PDC2 = PDC3 = PDC_70;

    /* 7) Activa control PWM sobre pines Low-side */
    OVDCONbits.POVD1L = 1;
    OVDCONbits.POVD2L = 1;
    OVDCONbits.POVD3L = 1;

    /* 8) Actualización inmediata del duty-cycle */
    PWMCON2bits.IUE = 1;

    /* 9) Enciende módulo PWM */
    PTCONbits.PTEN  = 1;
}

int main(void)
{
    __builtin_disable_interrupts();

    initBlink_T1();       // Timer 1  → LED 10 Hz (prioridad 5)
    initAllPWMs70();      // PWM1/2/3 al 70 %

    /* Timer 2 reservado: prioridad menor (3) si lo habilitas
       IPC1bits.T2IP = 3; */

    __builtin_enable_interrupts();

    while (1) { /* loop vacío */ }
    return 0;
}
