# dsPIC30F4011 Firmware Examples

A curated collection of basic firmware demos for the Microchip dsPIC30F4011, built with the XC-DSC 3.21 toolchain and a 20 MHz system clock. Each folder contains code illustrating a different peripheral or core feature:

- **0010_dspic30_led_test**  
  Simple LED blink projects using Timer 1 and interrupts, demonstrated on both internal and external clock sources.

- **0020_dspic30f_pwm**  
  Pulse-Width Modulation examples, including:  
  1. Initial PWM module setup  
  2. Driving multiple PWM channels at a fixed duty cycle  
  3. Smoothly ramping duty cycle up and down

- **0030_dspic30f_adc**  
  Analog-to-Digital Converter examples, such as:  
  1. Basic ADC initialization  
  2. ADC conversions via interrupt  
  3. Closed-loop control: adjusting PWM output based on ADC readings

## Getting Started

1. Open the desired example folder.  
2. Load the `.xcproj` into MPLAB X IDE.  
3. Build with the XC-DSC 3.21 compiler.  
4. Program your dsPIC30F4011 (20 MHz) and observe the behavior.

---

Feel free to browse each demo to learn how to configure and use the dsPIC30F4011’s core peripherals!```
