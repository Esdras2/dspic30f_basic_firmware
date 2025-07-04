/***********************************************************************
 *  Proyecto:    Firmware de ejemplo UART + ADC + LED de vida
 *  Archivo:     main.c
 *  Dispositivo: dsPIC30F4011 @ 7.37?MHz FRC + PLL×8 (Fosc ? 58.96?MHz)
 *               FCY = Fosc / 4 ? 14.74?MHz
 *  Autor:       Esdras Vázquez León
 *  Fecha:       25?Jun?2025
 *
 *  Descripción:
 *    - Configura el oscilador interno (FRC) con PLL×8.
 *    - Inicializa UART2 a 115?200?bps (8?N?1).
 *    - Muestra el canal AN0 con el ADC y envía la lectura (10?bits)
 *      mediante una cabecera 0xAA 0x55 seguida de los bytes [H] [L].
 *    - Parpadea un LED de vida cada 50?ms (placeholder ? Timer1).
 *
 *  Licencia: MIT (plantilla, reemplace según convenga).
 ***********************************************************************/

/*======================================================================*/
/*  CONFIGURATION BITS                                                  */
/*======================================================================*/

/*****************  CONFIGURATION BITS ? FRC + PLL×8  *******************/
/* 1) Oscilador primario:  FRC con PLL ×8                               */
/* 2) Oscilador secundario/Failsafe: FRC (sin cristal externo)          */
/* 3) Sin cambio dinámico de reloj ni monitor de reloj                  */
#pragma config FPR     = FRC_PLL8        // Primary Oscillator Mode (FRC w/ PLL×8)
#pragma config FOS     = PRI             // Oscillator Selection at Reset = Primary
#pragma config FCKSMEN = CSW_FSCM_OFF    // Clock Switching OFF, Fail?Safe OFF

#pragma config PWMPIN  = RST_PWMPIN      // PWM pins como IO tras reset
#pragma config LPOL    = PWMxL_ACT_HI
#pragma config HPOL    = PWMxH_ACT_HI

#pragma config WDT     = WDT_OFF         // Watchdog Timer OFF
#pragma config FPWRT   = PWRT_64         // Power?up Timer 64?ms
#pragma config BODENV  = BORV42          // Brown?out Voltage 4.2?V
#pragma config BOREN   = PBOR_OFF        // Brown?out Reset OFF
#pragma config MCLRE   = MCLR_EN         // MCLR pin enabled
#pragma config GWRP    = GWRP_OFF        // Code Write?Protection OFF
#pragma config GCP     = CODE_PROT_OFF   // Code Read?Protection OFF
#pragma config ICS     = ICS_PGD         // ICD communication pins (PGC/PGD)

#define _CRYSTAL_WORK   (7370000UL * 8UL)    // 7.37 MHz ×8
#define FCY             (_CRYSTAL_WORK/4UL)  // Fosc/2 según dsPIC30F (libpic30 usa Fosc/2)

/*======================================================================*/
/*  INCLUDES                                                            */
/*======================================================================*/
#include <xc.h>
#include <stdint.h>
#include <libpic30.h>

/*======================================================================*/
/*  DEFINES & MACROS                                                    */
/*======================================================================*/

/* Aplicaciones */
#define BAUD_RATE       115200UL                  // UART2 baud rate
#define ADC_CHANNEL     0                         // AN0
#define TIMER_LIFE_MS   50                        // Período parpadeo LED (ms)

/* Pines LED de vida (RD0) */
#define LIFE_LED_TRIS   TRISDbits.TRISD0
#define LIFE_LED_LAT    LATDbits.LATD0

/* ADC timing */
#define ADCS_TAD_COUNTS 4      // (ADCS + 1) = 5 ? Tad ? 340?ns @ 14.74?MHz
#define SAMPLING_TAD    10     // 10?Tad ? 3.4?µs tiempo de adquisición

/* UART2 pin mapping: RP21 ? U2TX, RP20 ? U2RX (ver datasheet PPS) */
#define U2TX_RP         21
#define U2RX_RP         20

/* Cálculo de BRG (modo BRGH=0, divisor 16) */
#define BRGVAL          (7)   // ? 7 @ 14.74?MHz

/*======================================================================*/
/*  VARIABLES GLOBALES                                                  */
/*======================================================================*/
static volatile uint16_t g_adc_raw = 0;    // Última muestra ADC (0?1023)

/*======================================================================*/
/*  PROTOTIPOS                                                          */
/*======================================================================*/
static void  system_init  (void);
static void  uart2_init   (void);
static void  uart2_putc   (char c);
static void  uart_send_pkt(uint16_t val);
static inline uint16_t reverse_bits_16(uint16_t num);

/*======================================================================*/
/*  IMPLEMENTACIÓN                                                      */
/*======================================================================*/

/*************************  UART2 **************************************/
static void uart2_init(void)
{
    /* Configura RF4=RX (entrada), RF5=TX (salida)    */
    TRISFbits.TRISF4 = 1;
    TRISFbits.TRISF5 = 0;

    U2MODE = 0;                  // Módulo apagado para configurar
    U2MODEbits.PDSEL = 0b00;     // 8?N?1
    U2MODEbits.STSEL = 0;

    U2BRG = (uint16_t)BRGVAL;    // Baud rate

    U2STA = 0;

    U2MODEbits.UARTEN = 1;       // 1) Enciende UART
    __delay_us(50);              // (opcional) espera estabilidad
    U2STAbits.UTXEN = 1;         // 2) Habilita transmisor

    /* Limpia flags */
    IFS1bits.U2RXIF = 0;
    IFS1bits.U2TXIF = 0;
}

static inline void uart2_putc(char c)
{
    while (U2STAbits.UTXBF);   // espera hueco en FIFO
    U2TXREG = (uint8_t)c;
}

/* Envío de paquete: 0xAA 0x55 [H] [L] */
static void uart_send_pkt(uint16_t val)
{
    uart2_putc(0xAA);
    uart2_putc(0x55);
    uart2_putc((uint8_t)(val >> 8));
    uart2_putc((uint8_t)val);
}

/*************************  ADC ***************************************/
static void adc_init(void)
{
    ADPCFG = 0xFFFF;            // Todos digitales ?
    ADPCFGbits.PCFG0 = 0;       // ? excepto AN0 (analógico)

    /* Configura tiempos */
    ADCON3bits.ADCS = ADCS_TAD_COUNTS; // Tad
    ADCON3bits.SAMC = SAMPLING_TAD;    // Tiempo de muestreo

    ADCHS = ADC_CHANNEL;        // MUX canal
    ADCON2 = 0;                 // Conversión única, sin búfer alterno

    ADCON1 = 0;                 // 10?bit, auto?convert
    ADCON1bits.FORM = 0b00;     // Entero ? signo?pos
    ADCON1bits.SSRC = 0b111;    // Gatillo interno (auto?conv)
    ADCON1bits.ASAM = 0;        // Muestreo manual

    /* Interrupción ADC */
    IEC0bits.ADIE  = 1;         // Habilita ISR
    IPC2bits.ADIP = 4;          // Prioridad intermedia
    IFS0bits.ADIF = 0;          // Limpia flag

    ADCON1bits.ADON = 1;        // Enciende módulo
}

void __attribute__((interrupt, auto_psv)) _ADCInterrupt(void)
{
    g_adc_raw = ADCBUF0;        // Guarda lectura
    IFS0bits.ADIF = 0;          // Limpia flag
}

/*************************  SISTEMA ***********************************/
static void system_init(void)
{
    /* LED de vida */
    LIFE_LED_TRIS = 0;          // Como salida
    LIFE_LED_LAT  = 0;

    /* Subsistemas */
    uart2_init();
    adc_init();

    /* TODO: Timer1 para parpadeo LED de vida */
}

/*************************  UTILIDADES ********************************/
static inline uint16_t reverse_bits_16(uint16_t num)
{
    uint16_t rev = 0;
    for (uint8_t i = 0; i < 16; ++i) {
        rev <<= 1;
        rev |= (num & 1);
        num >>= 1;
    }
    return rev;
}

/*======================================================================*/
/*  MAIN                                                                */
/*======================================================================*/
int main(void)
{
    __builtin_disable_interrupts();  // Bloquea IRQ globales
    system_init();
    __builtin_enable_interrupts();   // Activa IRQ globales

    while (1) {
        /* --- Inicia muestreo ADC --- */
        ADCON1bits.SAMP = 1;
        __delay_us(15);              // Tiempo mínimo de muestreo
        ADCON1bits.SAMP = 0;         // Inicia conversión
        while (!ADCON1bits.DONE);    // Espera fin de conversión

        uint16_t sample = g_adc_raw; // Lectura disponible (10 bits)
        uart_send_pkt(sample);       // Envía por UART

        __delay_ms(100);             // Ritmo de salida ? 10 Hz
    }

    /* Nunca llega aquí */
    return 0;
}
