// Ejemplo Mejorado: Parpadeo de LED con interrupción Timer1 (dsPIC30F4011 - FRC)

// Bits de Configuración
#pragma config FOS = FRC              // Oscilador interno FRC (~7.37 MHz)
#pragma config FCKSMEN = CSW_FSCM_OFF // Sin cambio ni monitoreo de reloj
#pragma config WDT = WDT_OFF          // Watchdog Timer desactivado
#pragma config FPWRT = PWRT_64        // Power-on Reset: 64 ms
#pragma config MCLRE = MCLR_EN        // Pin MCLR activo

#include <xc.h>
#include <libpic30.h>

// Frecuencia de ciclo de instrucción con FRC y PLL (opcional)
#define FCY 7370000UL

// Definición de pin LED
#define LED_TRIS TRISDbits.TRISD0
#define LED_LAT  LATDbits.LATD0

// Configuración Timer1 para interrupción cada 100ms (10Hz)
// PR1 = (FCY / Prescaler / Frecuencia deseada) - 1
// Ejemplo: (7.37MHz / 64 / 10) - 1 ? 11514
#define T1_PERIOD 11514

// Inicialización Timer1
void initTimer1(void) {
    T1CON = 0;                   // Limpia registro de control
    TMR1 = 0;                    // Inicializa contador Timer1
    PR1 = T1_PERIOD;             // Periodo para 100ms
    T1CONbits.TCKPS = 2;         // Prescaler 1:64
    T1CONbits.TCS = 0;           // Usa reloj interno (Fcy)

    IFS0bits.T1IF = 0;           // Limpia bandera interrupción
    IEC0bits.T1IE = 1;           // Habilita interrupción Timer1
    IPC0bits.T1IP = 4;           // Prioridad intermedia

    T1CONbits.TON = 1;           // Inicia Timer1
}

// Rutina de servicio de interrupción Timer1
void __attribute__((__interrupt__, auto_psv)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0;           // Limpia bandera interrupción
    LED_LAT ^= 1;                // Alterna LED
}

// Inicialización pin LED
void initLED(void) {
    LED_TRIS = 0;                // RD0 como salida
    LED_LAT = 0;                 // LED apagado inicialmente
}

// Función principal
int main(void) {
    // Deshabilita interrupciones para configuración inicial
    __builtin_disable_interrupts();

    initLED();
    initTimer1();

    // Habilita interrupciones globales
    __builtin_enable_interrupts();

    while(1) {
        // Lazo principal vacío, operación controlada por interrupción
    }
    return 0;
}
