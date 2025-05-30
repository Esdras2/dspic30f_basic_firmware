# dsPIC30F4011 Basic Firmware Collection

Este repositorio contiene ejemplos prácticos y comentados de firmware para el microcontrolador Microchip dsPIC30F4011, usando el compilador XC-DSC 3.21 y un cristal de 20 MHz. Cada carpeta aborda un periférico o función esencial del microcontrolador, facilitando el aprendizaje y la reutilización en proyectos reales.

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

## Cómo usar los ejemplos

1. Abre la carpeta del ejemplo que te interese.
2. Carga el archivo fuente `.c` en MPLAB X IDE (puedes crear un proyecto vacío y agregar el archivo).
3. Configura el microcontrolador dsPIC30F4011 y selecciona el compilador XC-DSC 3.21.
4. Compila y programa tu placa con un cristal de 20 MHz (o usa el oscilador interno según el ejemplo).
5. Observa el comportamiento en hardware (LED, salidas PWM, respuesta a señales analógicas).

## Agradecimientos

Este proyecto fue desarrollado con el apoyo y la orientación de la Academia de Ingeniería Electrónica del TecNM Campus Mazatlán. Agradecemos a los profesores y estudiantes que han contribuido con su experiencia y dedicación al fortalecimiento de la formación práctica en sistemas embebidos.

## Referencias

- Documentación oficial de Microchip dsPIC30F4011.
- Manuales y recursos de la Academia de Ingeniería Electrónica, TecNM Mazatlán.
- Ejemplos y notas de laboratorio desarrollados en el marco de las asignaturas de sistemas digitales y microcontroladores.

## Licencia

Este proyecto está bajo licencia MIT. Consulta [LICENSE](LICENSE) para más detalles.

---

Explora cada ejemplo y sus comentarios para aprender a configurar y aprovechar los periféricos clave del dsPIC30F4011 en tus propios desarrollos.