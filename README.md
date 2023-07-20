SilvaBack Digital Thermostat
---
## Features
1. Set temperature according to your personal preferences
2. Lock temperature once set it cannot be reset by another person
3. Auto-start to automatically start the thermostat incase the user forgets to turn it on
4. Remote control - control the thermostat using your smartphone

# Tech stack
## Software
1. ESP-IDF
2. MQTT
3. PlatformIO

## Hardware
1. ESP32 DOIT DEVKIT
2. NTC Thermistor
3. HVAC Switch
4. Rotary encoder
5. SSD1306 OLED screen
6. Battery powered - may change
7. KICAD - preferably

# Block diagram
![Block diagram](HomemadeThermostat.jpg)

# Temperature Measurement
###Thermistor
This circuit uses a 10k NTC thermometer to measure temperature. NTC thermistor's resistance
decreases with increase in temperature and vice versa. 

### Conversion
For conversion of resistance to analog voltage, we use a voltage divider circuit to measure voltage drop
across the known resistor value. We have configures the ADC to use 3.3V MAX and 12 bit. So its 
highest analog value is 4095. The formula below shows conversion of voltage-divider voltage to 
analog value:

```c
// voltage divider formula: Vo = (R1 / (R1 + R2) ) * Vin 
// where R1 is the know resistor, R2 is the thermistor resistance
// vo is the value read on the analog input pin
// Rearranging to get R2 as below
R2 = R1 * ((4095 / float(vo)) - 1.0);
```

To calculate the temperature, we use the SteinHart-Hart Equation (See reference below).
The function below will return the temperature in degrees Celcius.

```c
uint32_t calculate_temp_in_deg_C(uint32_t vo){
    /*
     * Calculate temperature in degrees
     */

    R2 = R1 * ((4095 / float(vo)) - 1.0); 

    log_R2 = log(R2);

    T = 1 / (A + B * log_R2 + C * log_R2 * log_R2 * log_R2);

    T = T - 273.15;

    return (uint32_t) T;

}
```

This formula is guaranteed to give us the desired temperature accurately.


# References
1. [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/index.html)
2. [Steinhart-Hart Equation for 10K thermistors](https://www.skyeinstruments.com/wp-content/uploads/Steinhart-Hart-Eqn-for-10k-Thermistors.pdf)