/*************************************************************
 *  dsPIC30F4011  – 20 MHz crystal  (FCY = 5 MHz)
 *  Toolchain     – XC-DSC 3.21
 *  Demo:  AN0 -> ADC -> actualiza PWM2H (pin RE3)
 *************************************************************/

#include <xc.h>
#include <libpic30.h>

/* ——— CONFIG BITS (idénticos a los que vienes usando) ——— */
/********************  CONFIGURATION BITS  ************************/
#pragma config FPR     = HS                 // HS cristal 20 MHz
#pragma config FOS     = PRI
#pragma config FCKSMEN = CSW_FSCM_OFF       // Sin cambio de reloj / sin monitor
#pragma config PWMPIN  = RST_PWMPIN          // PWM pins como IO después de reset
#pragma config LPOL    = PWMxL_ACT_HI
#pragma config HPOL    = PWMxH_ACT_HI
#pragma config WDT     = WDT_OFF
#pragma config FPWRT   = PWRT_64
#pragma config BODENV  = BORV42
#pragma config BOREN   = PBOR_OFF
#pragma config MCLRE   = MCLR_EN
#pragma config GWRP    = GWRP_OFF
#pragma config GCP     = CODE_PROT_OFF
#pragma config ICS     = ICS_PGD
/* ——————————————————— CONSTANTES DE DISEÑO —————————————————— */
/* Reloj derivado */
#define FCY             5000000UL          // Hz (usado por libpic30)

/* PWM deseado */
#define PWM_FREQ_HZ     5000UL             // 5 kHz
#define PTPER_COUNTS    ((FCY / PWM_FREQ_HZ) - 1)   // 5 000 000 / 5 000 – 1 = 999

/* ADC: TAD = (ADCS+1) × TCY   →  (4+1)×200 ns = 1 µs  */
#define ADCS_TAD_COUNTS 4
#define SAMPLING_TAD    10                 // 10 µs adquisición

/* ——————————————————— PROTOTIPOS ——————————————————— */
static void adc_select_pins(void);
static void adc_init_single_AN0(void);
static void pwm2_init_RE3(void);

/* ——————————————————— VARIABLES GLOBALES ————————————————— */
volatile unsigned int adc_value = 0;       // última muestra 0-1023
volatile unsigned int g_adc_raw = 0;


/* ——————————————————— SELECCIÓN DE PINES ADC ——————————— */
static void adc_select_pins(void)
{
    ADPCFG = 0xFFFF;          // Todos digitales …
    ADPCFGbits.PCFG0 = 0;     // … excepto AN0 (analógico)
}

/* ——————————————————— INICIALIZAR ADC (AN0) ———————————— */
static void adc_init_single_AN0(void)
{
    adc_select_pins();

    /* Tiempo: TAD y tiempo de muestreo */
    ADCON3bits.ADCS = ADCS_TAD_COUNTS; // (ADCS+1) = 5  → 1 µs
    ADCON3bits.SAMC = SAMPLING_TAD;    // 10 TAD  → 10 µs

    /* Canal único CH0 = AN0, referencias AVdd/AVss */
    ADCHS = 0;               // CH0+ = AN0
    ADCON2 = 0;              // Modo simple, un búfer

    /* Disparo interno: auto-convertir */
    ADCON1 = 0;
    ADCON1bits.FORM = 0;     // Resultado entero 16-bit
    ADCON1bits.SSRC = 0b111; // Fin de muestreo → inicio conversión
    ADCON1bits.ASAM = 1;     // Auto-sample tras cada conversión

    /* Interrupciones */
    IFS0bits.ADIF = 0;
    IEC0bits.ADIE = 1;       // Habilitar ADC ISR
    IPC2bits.ADIP = 4;       // Prioridad intermedia

    ADCON1bits.ADON = 1;     // ¡ADC andando!
}

/* ——————————————————— INICIALIZAR PWM2H EN RE3 ———————— */
static void pwm2_init_RE3(void)
{
    /* RE3 como salida digital */
    TRISEbits.TRISE3 = 0;

    /* Time-base común (PTCON) */
    PTCONbits.PTEN   = 0;    // Off mientras se programa
    PTCONbits.PTCKPS = 0;    // Pre-scaler 1:1 (TCY = 200 ns)
    PTCONbits.PTOPS  = 0;    // Post-scaler 1:1
    PTCONbits.PTMOD  = 0;    // Free-running
    PTPER = PTPER_COUNTS;    // 999 → periodo = (999+1)*2*TCY = 400 µs → 5 kHz

    /* Canal 2 independiente en RE3 */
    PWMCON1 = 0;
    PWMCON1bits.PMOD2 = 1;
    PWMCON1bits.PEN2H = 1;

    /* Sin dead-time ni sincronía */
    DTCON1  = 0;
    PWMCON2 = 0;

    /* Asegurar que el PWM controla la pata */
    OVDCON  = 0x0000;   // Todos los POVDx = 1  (PWM en control)

    /* Duty inicial */
    PDC2    = 0;
    OVDCONbits.POVD2H = 1;

    PTCONbits.PTEN = 1;      // Arranca PWM
}
/* ───────────── FUNCIONES AUXILIARES ───────────── */
static inline unsigned int adc_to_pdc(unsigned int adc10)
{
    /* Convierte 0…1023 → 0…PTPER_COUNTS con aritmética de 32 bits.
       Garantiza no sobrepasar PTPER. */
    unsigned long tmp = (unsigned long)adc10 * (PTPER_COUNTS + 2);
    tmp >>= 10;                               // ÷1024
    return (unsigned int)tmp;                 // 0…PTPER
}
/* ——————————————————— ADC INTERRUPT ——————————————————— */
void __attribute__((interrupt, auto_psv)) _ADCInterrupt(void)
{
    g_adc_raw = ADCBUF0;                      // guarda la última lectura
    PDC2      = adc_to_pdc(g_adc_raw);        // actualiza duty (0–100 %)
    IFS0bits.ADIF = 0;                // Limpia flag
}

/* ——————————————————— PROGRAMA PRINCIPAL ——————————————— */
int main(void)
{
    __builtin_disable_interrupts();    // 1· Bloquear IRQ

    pwm2_init_RE3();                   // 2· Time-base + canal PWM
    adc_init_single_AN0();             // 3· ADC free-running

    __builtin_enable_interrupts();     // 4· IRQ global ON

    /* 5· Bucle principal – aquí va tu lógica de alto nivel */
    for (;;)
    {
        /* NOP – placeholder */
        __builtin_nop();
    }
    /* Nunca se llega, pero estándar C exige return */
    return 0;
}