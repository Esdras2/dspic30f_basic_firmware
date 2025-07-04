/****************************************************************************************
 *  Proyecto   :  Firmware ?UART ? PWM?
 *  Archivo    :  uart_pwm_control.c
 *  Dispositivo:  dsPIC30F4011 ? 7.37?MHz FRC + PLL×8 ? Fosc ??58.96?MHz
 *                FCY = Fosc /?4 ??14.74?MHz
 *  Autor      :  Esdras Vázquez León
 *  Versión    :  0.3 ? 27?Jun?2025
 *
 *  Descripción ??????????????????????????????????????????????????????????????????????????
 *  Escucha paquetes de 4 bytes a través de UART2 con el formato:
 *        0xAA · 0x55 · [MSB] · [LSB]
 *  donde los dos últimos bytes codifican un valor de 10 bits (0?1023) que representa
 *  el duty?cycle deseado del canal PWM1L (RE0).
 *
 *  Si la nueva consigna difiere en ±4 cuentas con respecto al duty aplicado se
 *  actualiza el registro PDC1. La portadora PWM se genera a ~15?kHz (free?running,
 *  center?aligned).
 *
 *  ?Recursos HW
 *  ?????????????????????????????????????????????????????????????????????????????????????
 *  ? Reloj      : FRC + PLL×8  (??58.96?MHz)
 *  ? UART2      : 115?200 bps · 8?N?1 (RX & TX habilitado)
 *  ? PWM1L/RE0  : 15?kHz · duty proporcional 0?100?%
 *
 *  ?Herramientas probadas
 *  ?????????????????????????????????????????????????????????????????????????????????????
 *  ? MPLAB X IDE v6.20
 *  ? XC16 v3.21  (?mcpu=30F4011)
 *      Flags mínimos sugeridos:
 *          xc16-gcc  -mcpu=30F4011  -O1  -Wall  \
 *          -I"$XC16_INSTALL_DIR"/support/dsPIC30F/h  \
 *          -lpic30
 *    Habilite *?Link with Peripheral Library (libpic30)?* si emplea __delay_ms()/us().
 *
 *  Licencia ? MIT ? Puede reutilizar y modificar citando al autor.
 ****************************************************************************************/

/*========================================================================================*/
/*  CONFIGURATION BITS                                                                    */
/*========================================================================================*/
#pragma config FPR     = FRC_PLL8        // Primary Osc: FRC + PLL×8
#pragma config FOS     = PRI             // Oscillator Selection at Reset: Primary
#pragma config FCKSMEN = CSW_FSCM_OFF    // No clock?switching, no fail?safe

#pragma config PWMPIN  = RST_PWMPIN      // PWM pins como I/O tras reset
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

/*========================================================================================*/
/*  INCLUDES                                                                              */
/*========================================================================================*/
#define F_OSC           (7370000UL * 8UL)            // 7.37?MHz ×8 = 58.96?MHz
#define FCY             (F_OSC / 4UL)                // ??14.74?MHz (instrucción)
#include <xc.h>
#include <stdint.h>
#include <stdlib.h>     // abs()
#include <libpic30.h>   // __delay_ms()/us()

/*========================================================================================*/
/*  CONSTANTES ? AJUSTES DE TIME?BASE                                                     */
/*========================================================================================*/

/* UART */
#define UART_BAUD       115200UL
#define U2BRG_VAL       (7)  // BRGH = 0

/* PWM */
#define PWM_FREQ_HZ     15000UL                      // ~15?kHz
#define PTPER_VAL       (7)  // Prescaler = 1
#define DUTY_MAX        1023U                       // 10 bit resolution
#define DIFF_THRESHOLD  4U                          // Histéresis ±4 cuentas

/*========================================================================================*/
/*  VARIABLES GLOBALES                                                                    */
/*========================================================================================*/
static volatile uint16_t g_pwm_target  = 0;   // Último duty recibido (0?1023)
static volatile uint16_t g_pwm_current = 0;   // Duty aplicado

/* FSM recepción: 0?AA, 1?55, 2?[MSB], 3?[LSB] */
static volatile uint8_t  g_rx_state = 0;
static volatile uint16_t g_rx_word  = 0;

/*========================================================================================*/
/*  PROTOTIPOS DE FUNCIÓN                                                                  */
/*========================================================================================*/
static void     uart2_init(void);
static void     pwm1l_init(void);
static void     pwm1l_set_duty(uint16_t duty10);
static inline void uart2_putc(uint8_t c);
static void     uart2_puts(const char *s);

/*========================================================================================*/
/*  ISR ? UART2 (Recepción)                                                               */
/*========================================================================================*/
void __attribute__((interrupt, auto_psv)) _U2RXInterrupt(void)
{
    IFS1bits.U2RXIF = 0;                // Clear flag ASAP

    const uint8_t byte = U2RXREG;       // Leer FIFO (desborde ignorado: 1?byte)

    switch (g_rx_state)
    {
        case 0:
            g_rx_state = (byte == 0xAA) ? 1 : 0;
            break;

        case 1:
            g_rx_state = (byte == 0x55) ? 2 : 0;
            break;

        case 2:
            g_rx_word  = ((uint16_t)byte) << 8;  // MSB
            g_rx_state = 3;
            break;

        case 3:
            g_rx_word |= byte;                   // LSB
            g_pwm_target = g_rx_word & 0x03FF;   // 10 bit
            g_rx_state   = 0;
            break;

        default:
            g_rx_state = 0;
            break;
    }
}

/*========================================================================================*/
/*  UART2 ? Inicialización                                                                */
/*========================================================================================*/
static void uart2_init(void)
{
    /* I/O */
    TRISFbits.TRISF4 = 1;   // RF4 = RX
    TRISFbits.TRISF5 = 0;   // RF5 = TX

    /* MODE */
    U2MODE = 0;
    U2MODEbits.PDSEL = 0b00;   // 8?bits, sin paridad
    U2MODEbits.STSEL = 0;      // 1 stop
    U2BRG  = U2BRG_VAL;

    /* STA */
    U2STA = 0;
    U2MODEbits.UARTEN = 1;
    __delay_us(50);
    U2STAbits.UTXEN  = 1;

    /* RX Interrupt */
    IFS1bits.U2RXIF = 0;
    IEC1bits.U2RXIE = 1;
    IPC6bits.U2RXIP = 5;       // Prioridad media?alta
}

/*========================================================================================*/
/*  PWM1L (Motor Control PWM) ? Inicialización                                            */
/*========================================================================================*/
static void pwm1l_init(void)
{
    TRISEbits.TRISE0 = 0;      // RE0 como salida PWM1L

    /* Time?base ? Apagado para configurar */
    PTCON = 0;
    PTCONbits.PTCKPS = 0;      // Prescaler = 1
    PTCONbits.PTMOD  = 0;      // Free?running, center?aligned

    PTPER = PTPER_VAL;         // Periodo
    PWMCON1 = 0;

    PWMCON1bits.PMOD1 = 1;     // Modo independiente
    PWMCON1bits.PEN1L = 1;     // Habilita salida L

    DTCON1 = 0;                // Sin dead?time
    OVDCON = 0;                // Salida controlada por PWM
    OVDCONbits.POVD1L = 1;

    PWMCON2bits.IUE = 1;       // Update on PTMR rollover
    PTCONbits.PTEN  = 1;       // Arranca el PWM
}

/*========================================================================================*/
/*  PWM1L ? Conversión de duty (10 bits) ? ticks                                          */
/*========================================================================================*/
static inline void pwm1l_set_duty(uint16_t duty10)
{
    if (duty10 > DUTY_MAX) duty10 = DUTY_MAX;

    /* Duty ticks = (duty10 / 1023) * PTPER * 2 (center?aligned) */
    const uint32_t ticks = ((uint32_t)duty10 * PTPER_VAL) >> 9;  // ? *2 /1024
    PDC1 = (uint16_t)(ticks << 1);                               // Escala ×2
}

/*========================================================================================*/
/*  UART2 ? Utilidades de transmisión (debug opcional)                                    */
/*========================================================================================*/
static inline void uart2_putc(uint8_t c)
{
    while (U2STAbits.UTXBF);   // FIFO llena ? espera
    U2TXREG = c;
}

static void uart2_puts(const char *s)
{
    while (*s) uart2_putc((uint8_t)*s++);
}

/*========================================================================================*/
/*  MAIN                                                                                  */
/*========================================================================================*/
int main(void)
{
    __builtin_disable_interrupts();

    uart2_init();
    pwm1l_init();

    __builtin_enable_interrupts();

    uart2_puts("\r\nUART?PWM listo\r\n");

    for (;;) {
        /* Aplica duty si supera histéresis */
        if (abs((int)g_pwm_target - (int)g_pwm_current) > DIFF_THRESHOLD) {
            pwm1l_set_duty(g_pwm_target);
            g_pwm_current = g_pwm_target;
        }
        __delay_ms(1);     // Idle mínimo (? 1?kHz loop)
    }

    /* No debería alcanzarse */
    return 0;
}
