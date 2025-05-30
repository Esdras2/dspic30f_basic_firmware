# dsPIC30F4011 Basic Firmware Collection

Este repositorio ofrece una colección de ejemplos prácticos y detalladamente comentados para el microcontrolador Microchip dsPIC30F4011, utilizando el compilador XC-DSC 3.21 y un cristal de 20 MHz. El objetivo es proporcionar una base sólida para el aprendizaje y desarrollo de firmware, cubriendo la configuración y uso de los periféricos más relevantes del dsPIC30F4011 en aplicaciones reales.

Cada carpeta del proyecto aborda un periférico o función clave, facilitando la comprensión modular y la reutilización del código en nuevos desarrollos.

---

## Estructura del Proyecto

- **0010_dspic30_led_test/**
  - Ejemplos de parpadeo de LED usando Timer1 e interrupciones.
  - Incluye variantes con oscilador interno (FRC) y cristal externo (HS).
  - Ideal para aprender configuración básica de pines, temporizadores y manejo de interrupciones.

- **0020_dspic30f_pwm/**
  - Demostraciones del módulo PWM:
    - `10_initial_pwm_main.c`: Configuración mínima para generar PWM en un canal.
    - `20_all_70_pwm_main.c`: Genera señal PWM al 70% en tres canales simultáneamente.
    - `30_pwm_main.c`: Ejemplo de rampa de ciclo útil (duty cycle) que sube y baja automáticamente cada 2 segundos.
  - Incluye [note.md](0020_dspic30f_pwm/note.md) con pasos detallados para configurar PWM correctamente en dsPIC30F4011.

- **0030_dspic30f_adc/**
  - Ejemplos de uso del convertidor analógico-digital (ADC):
    - `11_initial_adc_config.c`: Inicialización básica del ADC y lectura continua mediante interrupción.
    - `20_adc_pwm_main.c`: Controla el ciclo útil de un PWM en función de la lectura analógica (AN0), usando interrupciones.
    - `21_adc_pwm_internal_osc.c`: Variante que usa el oscilador interno para el mismo control ADC→PWM.
  - Incluye [note.md](0030_dspic30f_adc/note.md) con notas sobre prioridades de interrupción y control de PWM desde el ADC.

---

## Cómo usar los ejemplos

1. Abre la carpeta del ejemplo que te interese.
2. Carga el archivo fuente `.c` en MPLAB X IDE (puedes crear un proyecto vacío y agregar el archivo).
3. Configura el microcontrolador dsPIC30F4011 y selecciona el compilador XC-DSC 3.21.
4. Compila y programa tu placa con un cristal de 20 MHz (o usa el oscilador interno según el ejemplo).
5. Observa el comportamiento en hardware (LED, salidas PWM, respuesta a señales analógicas).

---

## Glosario

- **Prescaler**: Divisor de frecuencia que permite ajustar la velocidad a la que un temporizador cuenta. Elegir el prescaler adecuado permite obtener periodos de temporización precisos sin desbordar el registro del temporizador.
- **Duty Cycle (Ciclo útil)**: Porcentaje del tiempo en que una señal PWM permanece en nivel alto durante un ciclo. Controla la potencia promedio entregada a una carga.
- **ISR (Interrupt Service Routine)**: Rutina de servicio de interrupción. Es una función especial que se ejecuta automáticamente cuando ocurre un evento de hardware, como el desbordamiento de un temporizador o la finalización de una conversión ADC.
- **FRC (Fast RC Oscillator)**: Oscilador interno rápido del dsPIC30F4011 (~7.37 MHz), útil para aplicaciones donde no se requiere alta precisión de reloj externo.
- **PTPER**: Registro que define el periodo de la señal PWM. Su valor depende de la frecuencia deseada y del prescaler seleccionado.

---

## ¿Por qué estas configuraciones?

- **Prescaler**: Se selecciona para que el temporizador pueda alcanzar el periodo deseado sin desbordarse. Un prescaler mayor permite periodos más largos, pero reduce la resolución.
- **Duty Cycle**: Ajustar el ciclo útil permite controlar la energía promedio en aplicaciones como control de motores o brillo de LEDs.
- **Interrupciones**: Usar ISRs permite que el microcontrolador responda rápidamente a eventos sin necesidad de estar verificando constantemente (polling), mejorando la eficiencia.
- **Oscilador interno vs externo**: El FRC es conveniente para pruebas rápidas o aplicaciones donde la precisión no es crítica. El cristal externo se recomienda para aplicaciones que requieren mayor estabilidad y precisión de reloj.

---

## Referencias y Recursos

- [Hoja de datos dsPIC30F4011](https://ww1.microchip.com/downloads/en/DeviceDoc/70138F.pdf)
- [Manual de referencia de la familia dsPIC30F](https://ww1.microchip.com/downloads/en/DeviceDoc/70046E.pdf)
- [Guía de configuración de periféricos dsPIC30F](https://www.microchip.com/en-us/product/dsPIC30F4011)
- Documentación oficial de Microchip dsPIC30F4011.
- Manuales y recursos de la Academia de Ingeniería Electrónica, TecNM Mazatlán.
- Ejemplos y notas de laboratorio desarrollados en el marco de las asignaturas de sistemas digitales y microcontroladores.

---

## Agradecimientos

Este proyecto fue desarrollado con el apoyo y la orientación de la Academia de Ingeniería Electrónica del TecNM Campus Mazatlán. Agradecemos a los profesores y estudiantes que han contribuido con su experiencia y dedicación al fortalecimiento de la formación práctica en sistemas embebidos.

---

## Licencia

Este proyecto está bajo licencia MIT. Consulta [LICENSE](LICENSE) para más detalles.

---

Explora cada ejemplo y sus comentarios para aprender a configurar y aprovechar los periféricos clave del dsPIC30F4011 en tus propios desarrollos.