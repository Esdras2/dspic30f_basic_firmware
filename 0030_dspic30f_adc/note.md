# Notas sobre el uso del ADC en dsPIC30F4011

Ejemplos desarrollados para dsPIC30F4011, compilador XC-DSC 3.21, cristal de 20 MHz.

## Contenido de los ejemplos

1. **Configuración inicial del ADC**
   - Ejemplo de inicialización básica del módulo ADC.
   - Selección de canal analógico, configuración de pines y ajuste de registros para conversión continua.

2. **Prioridad de interrupciones del ADC**
   - Cómo establecer la prioridad de la interrupción del ADC para asegurar la respuesta oportuna a nuevas conversiones.
   - Ejemplo de rutina de servicio de interrupción (ISR) para leer el resultado del ADC.

3. **Control de PWM mediante el ADC por interrupción**
   - Uso de la lectura analógica para modificar el ciclo útil (duty cycle) de una señal PWM en tiempo real.
   - Ejemplo de lazo cerrado: el valor leído por el ADC ajusta directamente la salida PWM.
   - Incluye variantes usando oscilador interno y externo.

## Recomendaciones

- Verifica la configuración de los pines analógicos (ANx) y la referencia de voltaje.
- Ajusta la prioridad de interrupción del ADC según la aplicación.
- Consulta los comentarios en el código fuente para detalles de cada paso.

---