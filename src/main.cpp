#include <Arduino.h>
#include <driver/adc.h>
#include "defines.h"
#include "Button.h" /* Handle button presses */
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// surrounding temperature
uint32_t ambient_temperature = 0;

/**
 * Create keypads
 * Menu
 * Up
 * Down
 * Reset
 */
Button menu_btn = Button(25, HIGH);
//Button up_btn(33, HIGH);
//Button down_btn(39, HIGH);
//Button reset_btn(34, HIGH);

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

/**
 * Configure the screen
 */

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void config_screen(){
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)){
        debugln("[-]ERR: Display allocation failed!");
        for(;;);
    }

    // allocation succeeded
    display.clearDisplay();
}

void display_default(){
    // Display Text
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,28);
    display.println("Hello world!");
    display.display();
    delay(2000);
    display.clearDisplay();
}

void setup() {
    Serial.begin(BAUD_RATE);

    // configure ADC
    config_adc_rtc();

    // configure screen
    config_screen();

    display_default();

}

void loop() {

    ambient_temperature = read_ambient_temperature();
    if (ambient_temperature == PARAMETER_ERR){
        debugln("Err:Could not read");
    }

    debugln(ambient_temperature);

    // poll the keypad buttons
    if(menu_btn.getState() == Button::Pressed){
        debugln("menu");
    }

}