/**
 * Handle temperature measurement
 */

#include "Temperature.h"
#include <Arduino.h>

float A = 1.125308855e-03;
float B =  0.234711863e-03;
float C = 0.000085663e-03;

uint32_t analog_out;
float R1 = 10000;
float R2, log_R2, T;

uint32_t calculate_temp_in_deg_C(uint32_t vo){
    /*
     * Calculate temperature in degrees
     */

    R2 = R1 * ((4095 / float(vo)) - 1.0); //voltage division. 4095 reps 3.3V for ESP32 12 bit ADC

    log_R2 = log(R2);

    T = 1 / (A + B * log_R2 + C * log_R2 * log_R2 * log_R2);

    T = T - 273.15;

    return (uint32_t) T;

}


