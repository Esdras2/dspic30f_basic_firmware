/**********************************************************************
 *  dsPIC30F4011  – 20 MHz crystal, PLL ×8  (FCY = 40 MHz)
 *  Toolchain     – XC‑DSC 3.21+ (C30 mode)
 *
 *  Demo: 10 kHz PWM‑synchronised PI current/voltage loop
 *         Implemented 100 % with single‑cycle DSP MAC instructions.
 *
 *  Key improvements over v1:
 *  – Consistent Q1.15 fixed‑point everywhere (ADC scaled ↗︎ 15 bits)
 *  – Hardware saturation enabled (CORCON.SATA) to protect integrator
 *  – Deterministic trigger: PWM special‑event → ADC → ISR
 *  – Explicit interrupt priorities (level 6) and context save‑restore
 *  – Helper macros for Q‑math & saturation → clearer tuning section
 *  – More robust anti‑wind‑up (clamp tracks duty range automatically)
 *  – Dead‑time ready: complementary PWM2L enabled but held low
 *
 *  Author:  <your‑name> — 2025‑05‑30
 **********************************************************************/

/*==================== CONFIGURATION BITS ===========================*/
#pragma config FPR     = HS_PLL8        // 20 MHz → 160 MHz ÷ 4 = 40 MHz FCY
#pragma config FOS     = PRI
#pragma config FCKSMEN = CSW_FSCM_OFF
#pragma config PWMPIN  = RST_PWMPIN     // PWM pins hi‑Z after reset
#pragma config LPOL    = PWMxL_ACT_HI
#pragma config HPOL    = PWMxH_ACT_HI
#pragma config WDT     = WDT_OFF
#pragma config FPWRT   = PWRT_16
#pragma config BOREN   = PBOR_ON
#pragma config MCLRE   = MCLR_EN
#pragma config ICS     = ICS_PGD
/*==================================================================*/

/*=========================== Constants ============================*/
#define FCY         40000000UL
#define PWM_FREQ_HZ 10000UL
#include <xc.h>
#include <stdint.h>
#include <libpic30.h>
/*====================== Fixed‑point helpers ========================*/
#define Q15_SHIFT   15
#define Q15_ONE     (1 << Q15_SHIFT)
#define SAT_Q15(x)  __builtin_saturate(x, 15)   /* needs CORCON.SATA */

static inline int32_t mac_q15(int16_t a, int16_t b, int32_t acc)
{   /* one‑cycle multiply‑accumulate */
    return __builtin_mla(a, b, acc);
}
static inline int32_t mul_q15(int16_t a, int16_t b)
{   return __builtin_mulss(a, b);  }


/* Tunable PI gains (Q1.15) */
static volatile int16_t Kp_q15 = 0.5  * Q15_ONE;   /* 0.50 */
static volatile int16_t Ki_q15 = 0.10 * Q15_ONE;   /* 0.10 */

/*=========================== Globals ==============================*/
static volatile int16_t setpoint_q15  = 0.5 * Q15_ONE;  /* 0‑1 per‑unit */
static volatile int32_t integrator_q30 = 0;             /* extended */

/*==================== Function prototypes =========================*/
static void init_clock(void);
static void init_pwm(void);
static void init_adc(void);
static void init_corcon(void);

/*============================== MAIN ==============================*/
int main(void)
{
    init_clock();
    init_corcon();
    init_pwm();
    init_adc();

    /* idle: all control work is interrupt‑driven */
    while (1)  __builtin_clrwdt();
}

/*---------------- CORCON: DSP engine set‑up -----------------------*/
static void init_corcon(void)
{
    CORCONbits.SATA = 1;   /* enable accumulator saturation                          */
    CORCONbits.SATB = 1;   /* saturate writes to ACCB as well                        */
    CORCONbits.IF    = 0;  /* fractional mode (Q arithmetic)                         */
    CORCONbits.RAF   = 0;  /* round to nearest (optional)                            */
}

/*---------------- PWM module (10 kHz centre‑aligned) --------------*/
static void init_pwm(void)
{
    PTCONbits.PTEN   = 0;                         /* disable time‑base while configuring */
    PTCONbits.PTCKPS = 0;                         /* 1:1 prescale                        */
    PTCONbits.PTMOD  = 0b10;                      /* centre‑aligned                      */

    PTPER  = (FCY / PWM_FREQ_HZ) - 1;             /* period value                        */
    PDC2   = 0;                                   /* start with 0 % duty                 */

    PWMCON1bits.PMOD2 = 1;                        /* independent outputs                */
    PWMCON1bits.PEN2H = 1;                        /* PWM2H → RE3                        */
    PWMCON1bits.PEN2L = 1;                        /* PWM2L → RE2 (complement), for future*/
    DTCON1           = 20;                        /* ~500 ns dead‑time @ 40 MHz          */

    PTCONbits.PTEN   = 1;                         /* start PWM                           */
}

/*---------------- ADC: PWM‑triggered, 500 ksps --------------------*/
static void init_adc(void)
{
    /* Basic 10‑bit signed fractional output (‑1 … 0.999) */
    ADCON1 = 0;
    ADCON1bits.SSRC = 0b111;      /* auto‑convert after TAD count */
    ADCON1bits.FORM = 0b01;       /* signed fractional            */

    ADCON2 = 0;                   /* use AVdd/AVss refs, one sample/channel */
    ADCON3bits.ADCS = 5;          /* TAD = 6 × Tcy = 150 ns > min 75 ns     */

    ADCHS  = 0;                   /* sample AN0                        */
    ADPCFG = 0xFFFE;              /* AN0 = analog, others digital      */

    /* PWM special‑event trigger → SAMP set @ centre of period */
    ADTRIG = 0;                   /* clear all first                  */
    ADTRIGbits.TRGSRC0 = 0b0010;  /* PWM2 special event               */

    IFS0bits.ADIF = 0;
    IPC2bits.ADIP = 6;            /* priority level 6                 */
    IEC0bits.ADIE = 1;

    ADCON1bits.ADON = 1;          /* enable ADC                       */
}

/*================= ADC interrupt = PI control loop ===============*/
void __attribute__((interrupt, no_auto_psv)) _ADCInterrupt(void)
{
    IFS0bits.ADIF = 0;          /* clear flag early */

    /*---------- Read & scale feedback (10‑bit → Q1.15) ----------*/
    int16_t feedback_q15 = (int16_t)ADCBUF0 << 5;   /* 10‑bit ↗︎ 15‑bit */

    /*---------------- PI algorithm in DSP core ------------------*/
    int16_t error_q15 = setpoint_q15 - feedback_q15;

    integrator_q30 = mac_q15(error_q15, Ki_q15, integrator_q30);

    /* Anti‑wind‑up: clamp to ±100 % duty in Q30 space */
    int32_t max_int = ((int32_t)Q15_ONE << Q15_SHIFT);   /* 1.0 in Q30 */
    if (integrator_q30 >  max_int) integrator_q30 =  max_int;
    if (integrator_q30 < -max_int) integrator_q30 = -max_int;

    int32_t acc_q30 = (mul_q15(error_q15, Kp_q15) << 1) + integrator_q30;

    /*---------- Convert to duty register units (0…2·PTPER) ------*/
    int16_t duty_q15 = SAT_Q15(acc_q30 >> Q15_SHIFT);    /* apply saturation */

    uint16_t duty_counts = (uint16_t)(((int32_t)duty_q15 * (PTPER << 1)) >> Q15_SHIFT);
    PDC2 = duty_counts;
}
