#include <Arduino.h>
#include <driver/adc.h>
#include "defines.h"

uint32_t ambient_temperature = 0;

/**
 * Configure ADC_RTC
 */
void config_adc_rtc(){
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
}

uint32_t read_ambient_temperature(){
    uint32_t val = adc1_get_raw(ADC1_CHANNEL_0);
    return val;
}

void setup() {
    Serial.begin(BAUD_RATE);

    config_adc_rtc();

}

void loop() {

    ambient_temperature = read_ambient_temperature();
    if (ambient_temperature == PARAMETER_ERR){
        debugln("Err:Could not read");

    }

    debugln(ambient_temperature);
}