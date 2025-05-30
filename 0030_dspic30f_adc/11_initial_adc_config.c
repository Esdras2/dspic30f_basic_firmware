/*************************************************************
 *  dsPIC30F4011  – 20 MHz crystal  (FCY = 5 MHz)
 *  Toolchain     – XC-DSC 3.21
 *
 *  Purpose: demonstrate a bare-bones main() that
 *           – powers up peripherals
 *           – enables interrupts
 *           – idles while ADC ISR streams data
 *************************************************************/

#include <xc.h>
#include <libpic30.h>

/* ——— CONFIG BITS (match those you already use) ——— */
#pragma config FPR     = HS             // 20 MHz crystal
#pragma config FOS     = PRI
#pragma config FCKSMEN = CSW_FSCM_OFF
#pragma config PWMPIN  = RST_IOPIN
#pragma config WDT     = WDT_OFF
#pragma config FPWRT   = PWRT_64
#pragma config BOREN   = PBOR_ON
#pragma config MCLRE   = MCLR_EN
#pragma config ICS     = ICS_PGD


/* ——— forward declaration from previous snippet ——— */
void adc_init_single_AN0(void);

/* ——— global to receive the latest ADC sample ——— */
volatile unsigned int adc_value = 0;


/* ---------- compile-time choices ---------- */
#define ADC_CHAN_AN0   0        // CH0+ = AN0, CH0- = Vref–
#define ADCS_TAD_COUNTS 4       // (ADCS+1) → TAD = 5 × 200 ns = 1 µs
#define SAMPLING_TAD    10      // sample for 10 × TAD  (≈10 µs)

/* ---------- pin map: make AN0 analogue, the rest digital ---------- */
static void adc_select_pins(void)
{
    ADPCFG = 0xFFFF;           // everything digital first
    ADPCFGbits.PCFG0 = 0;      // AN0 back to analogue
}

/* ---------- ADC initialisation ---------- */
void adc_init_single_AN0(void)
{
    adc_select_pins();

    /*--- timing ---*/
    ADCON3bits.ADCS = ADCS_TAD_COUNTS - 1;   // TAD ≥ 83 ns ; here 1 µs 
    ADCON3bits.SAMC = SAMPLING_TAD;          // 10 TAD acquisition (Meets 10-bit spec)

    /*--- channel and buffer settings ---*/
    ADCHS  = ADC_CHAN_AN0;      // CH0 positive input = AN0, negative = Vref–
    ADCON2 = 0;                 // one buffer, one channel group, AVdd/AVss refs

    /*--- conversion trigger: internal counter (auto-convert) ---*/
    ADCON1 = 0;
    ADCON1bits.FORM = 0;        // integer, right-justified
    ADCON1bits.SSRC = 0b111;    // end sample / start convert automatically
    ADCON1bits.ASAM = 1;        // auto-sample immediately after last convert

    /*--- interrupt every sample ---*/
    IFS0bits.ADIF  = 0;
    IEC0bits.ADIE  = 1;
    IPC2bits.ADIP  = 4;         // priority 4 (tune to suit)

    ADCON1bits.ADON = 1;        // power-up ADC – sampling starts now
}

/* ——— ADC interrupt moves each sample into adc_value ——— */
void __attribute__((interrupt, auto_psv)) _ADCInterrupt(void)
{
    adc_value = ADCBUF0;   // grab result for main context
    IFS0bits.ADIF = 0;     // clear flag
}

/* ——— MAIN ——— */
int main(void)
{
    /* 1. Block interrupts while you touch core registers */
    __builtin_disable_interrupts();

    /* 2. Initialise the ADC (AN0 free-running, interrupt every sample) */
    adc_init_single_AN0();

    /* 3. Global interrupt enable */
    __builtin_enable_interrupts();

    /* 4. Main loop — replace with your application logic */
    for (;;)
    {
        /* Example placeholder: read the latest value and do something */
        unsigned int local_copy = adc_value;   // access with data coherency
        (void)local_copy;                      // prevent “unused” warning

        /* Low-priority background tasks go here */
    }

    /* The MCU never reaches this, but C requires a return. */
    return 0;
}
