/**
 * Handle temperature measurements
 */

#include <Arduino.h>

#ifndef SILVABACK_THERMOSTAT_TEMPERATURE_H
#define SILVABACK_THERMOSTAT_TEMPERATURE_H

// Steinhart-Hart coefficients for a 10k thermistor
extern float A;
extern float B;
extern float C;

extern uint32_t analog_out;
extern float R1;
extern float R2, log_R2, T;

uint32_t calculate_temp_in_deg_C(uint32_t);

#endif //SILVABACK_THERMOSTAT_TEMPERATURE_H
