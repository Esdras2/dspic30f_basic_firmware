# Notas sobre el uso del módulo PWM en dsPIC30F4011

Ejemplos desarrollados para dsPIC30F4011 en C, usando el compilador XC-DSC 3.21 y cristal externo HS de 20 MHz.

## Descripción general

- Salida principal: PWM3L (RE4).
- El ejemplo principal genera una señal PWM cuyo ciclo útil (duty cycle) aumenta y disminuye automáticamente cada 2 segundos (configurable).
- Incluye ejemplos de configuración básica y generación de PWM en múltiples canales.

## Pasos recomendados para configurar el PWM

| #  | Acción | Qué hacer | Por qué es importante |
|----|--------|-----------|----------------------|
| 1  | Configura los pines como salidas digitales | Establece todos los pines como digitales con `ADPCFG = 0xFFFF;`, luego limpia el bit TRIS correspondiente a cada pin PWM (ej. `TRISEbits.TRISE0 = 0;`). | Evita que el ADC u otros periféricos interfieran con el pin. |
| 2  | Deshabilita el time-base del PWM mientras lo programas | `PTCONbits.PTEN = 0;` detiene el contador para evitar glitches al modificar registros. | Garantiza que los cambios de configuración no generen señales erróneas. |
| 3  | Define la frecuencia del PWM | Selecciona un prescaler (`PTCKPS`) y carga `PTPER` con:<br>`PTPER = FCY / (2 × fPWM) – 1`<br>Ejemplo: para 10 kHz y FCY = 5 MHz, `PTPER = 249`. | Determina la frecuencia de la señal PWM. |
| 4  | Selecciona modo independiente y habilita los canales deseados | Para cada canal: `PMODx = 1` (modo independiente) y activa `PENxL` o `PENxH`.<br>Ejemplo para PWM1 low-side:<br>`PWMCON1bits.PMOD1 = 1;`<br>`PWMCON1bits.PEN1L = 1;`<br>(Opcional) Desactiva dead-time si no usas medio puente: `DTCON1 = 0;`. | Permite controlar cada canal de forma individual y evita retardos innecesarios. |
| 5  | Carga el ciclo útil (duty cycle) | Convierte el porcentaje a unidades de registro:<br>`PDCx = (PTPER + 1) × 2 × Duty% / 100`<br>Ejemplo: 70% con `PTPER = 249` da `PDC1 = 350`. | Ajusta la proporción de tiempo en alto/bajo de la señal PWM. |
| 6  | Permite actualizaciones inmediatas del duty cycle | `PWMCON2bits.IUE = 1;` habilita la actualización instantánea del duty cycle. | Hace que los cambios en tiempo real sean suaves y sin retardo. |
| 7  | Entrega el control del pin al módulo PWM | Configura los bits de `OVDCON`, ej.:<br>`OVDCONbits.POVD1L = 1;` // PWM controla RE0 | Sin esto, el pin sigue como GPIO y no se genera la señal PWM. |
| 8  | Inicia el módulo PWM | `PTCONbits.PTEN = 1;` activa el contador y la señal PWM aparece en el pin configurado. | Comienza la generación de la señal PWM. |

## Notas adicionales

- Puedes ajustar el periodo de rampa modificando el temporizador o el retardo en el código.
- Consulta los comentarios en el código fuente para detalles específicos de cada ejemplo y canal.
- Si usas cargas inductivas o medio puente, revisa la configuración de dead-time.

---