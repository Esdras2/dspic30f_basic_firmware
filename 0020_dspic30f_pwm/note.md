
dspic30f4011 in C, XC_DSC 3.21V

external osc HS / 20Mhz

output
PWM PWM3L/RE4


every 2 secong (configurable)
increce a decrece pwm

#	Action	What You Do	Why It Matters
## 1 Configure the pins as plain digital outputs
Set every pin to digital with ADPCFG = 0xFFFF;, then clear the TRIS bit for each PWM pin you’ll drive (e.g. TRISEbits.TRISE0 = 0). This keeps the ADC and other peripherals from stealing the pin.

## 2 Disable the PWM time-base while you program it
PTCONbits.PTEN = 0; freezes the counter so changes to period or duty registers cannot produce glitches.

## 3 Define the PWM frequency
Pick a prescaler (PTCKPS) and load PTPER with

PTPER = FCY / (2 × fPWM) – 1
For a 10 kHz waveform at FCY = 5 MHz, the result is 249.

## 4 Select independent mode and enable the desired channel(s)
For each channel: set PMODx = 1 (independent) and the corresponding PENxL or PENxH = 1. Example for PWM1 low-side:

PWMCON1bits.PMOD1 = 1;
PWMCON1bits.PEN1L = 1;
(Optional) Disable dead-time if you are not driving a half-bridge
DTCON1 = 0; suffices for LEDs, resistive loads, or single MOSFETs.

## 5 Load the duty cycle
Convert your percentage into register units with

PDCx = (PTPER + 1) × 2 × Duty% / 100
A 70 % duty cycle with PTPER = 249 yields PDC1 = 350.

## 6 Allow duty-cycle writes to update immediately
PWMCON2bits.IUE = 1; removes the one-period delay, making real-time adjustments smooth.

## 7 Hand control of the pins to the PWM module
Set the relevant OVDCON bits, e.g.

OVDCONbits.POVD1L = 1;   // PWM takes charge of RE0
Without this step, the pin remains an ordinary GPIO and no waveform appears.

## 8 Start the PWM engine
PTCONbits.PTEN = 1; lets the counter run and the configured waveform immediately shows up on the pin.