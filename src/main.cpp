#include <Arduino.h>
#include <driver/adc.h>
#include "defines.h"
#include "Button.h" /* Handle button presses */
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMono9pt7b.h>

// surrounding temperature
uint32_t ambient_temperature = 0;
uint32_t set_point = 40;

// default mode = HOME
uint8_t state = HOME;

/*menu items */
char *menu[MENU_SIZE] = {
        "Set temperature",
        "Enable remote",
        "Lock temperature",
        "Reset"
};

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

uint16_t wifiHotspot_icon_W = 16;
uint16_t wifiHotspot_icon_H = 16;

static const unsigned char PROGMEM home_icon16x16[] ={
                0b00000111, 0b11100000, // ######
                0b00001111, 0b11110000, // ########
                0b00011111, 0b11111000, // ##########
                0b00111111, 0b11111100, // ############
                0b01111111, 0b11111110, // ##############
                0b11111111, 0b11111111, // ################
                0b11000000, 0b00000011, // ## ##
                0b11000000, 0b00000011, // ## ##
                0b11000000, 0b00000011, // ## ##
                0b11001111, 0b11110011, // ## ######## ##
                0b11001111, 0b11110011, // ## ######## ##
                0b11001100, 0b00110011, // ## ## ## ##
                0b11001100, 0b00110011, // ## ## ## ##
                0b11001100, 0b00110011, // ## ## ## ##
                0b11111100, 0b00111111, // ###### ######
                0b11111100, 0b00111111, // ###### ######
        };

static const unsigned char PROGMEM wifiHotspot_icon[] = {
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
        B00100000, B00000100,
        B01001000, B00010010,
        B01010010, B01001010,
        B01010100, B00101010,
        B01010101, B10101010,
        B01010101, B10101010,
        B01010100, B00101010,
        B01010010, B01001010,
        B01001000, B00010010,
        B00100000, B00000100,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000
};
void config_screen(){
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)){
        debugln("[-]ERR: Display allocation failed!");
        for(;;);
    }

    // allocation succeeded
    display.clearDisplay();
}

void display_default(uint32_t val, uint32_t set_point){
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.drawBitmap(0, 0, home_icon16x16, 10, 10, WHITE);
    display.setCursor(13,0);
    display.println("Online");
    display.drawRect(30, 17, 90, 35, WHITE);

    // units
    display.setCursor(50, 0);
    display.println("Mode: Auto");

    // show the set reference temperature
    display.setCursor(0, 17);
    display.println("Set: ");
    display.setCursor(0, 25);
    display.println(set_point);
    display.drawCircle(16, 27, 1, WHITE);
    display.setCursor(18, 25);
    display.println("C"); // todo: function to change units and convert

    // draw logo
    display.drawBitmap(0, 35, wifiHotspot_icon, 15, 15, WHITE);

    // display the temperature value
//    display.setTextSize(3);
    display.setFont(&FreeMonoBold18pt7b);
    display.setCursor(34, 43);
    display.println(val / 100); // todo: modulo just for testing
    display.drawCircle(80, 22, 2, WHITE);
    display.setCursor(83, 43);
    display.println("C");
    display.setFont();

    // show menu options at the bottom
    display.setTextSize(1);
    display.setCursor(0, SCREEN_HEIGHT-10);
    display.println("July, Thur 12:34 AM");

    display.display();

}

void display_static_menu(){
    /**
     * display static menu
     */
     display.clearDisplay();
//     display.setFont(&FreeMono9pt7b);
     display.setCursor(15, 0);
     display.println("Menu");

     /*========================== */
     display.setCursor(0, 17);
     display.println(">");

    for (int i = 0; i < MENU_SIZE; ++i) {
        display.setCursor(MENU_X_OFFSET, MENU_Y_OFFSET);
        display.println(menu[i]);

        MENU_Y_OFFSET += 10;

    }

    MENU_Y_OFFSET = 17;

     display.display();

}

void setup() {
    Serial.begin(BAUD_RATE);

    // configure ADC
    config_adc_rtc();

    // configure screen
    config_screen();

    // set default mode
    display_default(ambient_temperature, set_point);

}

void loop() {

    ambient_temperature = read_ambient_temperature();
    if (ambient_temperature == PARAMETER_ERR){
        debugln("Err:Could not read");
    }

    debugln(ambient_temperature);

    /* toggle state */
    if((menu_btn.getState() == Button::Pressed) && (state == HOME)){
        state = MENU;

    } else if ((menu_btn.getState() == Button::Pressed) && (state == MENU)){
        state = HOME;

    }

    /* display either menu or home page */
    if ((menu_btn.getState() == Button::NotPressed) && (state == MENU)){
        display_static_menu();
        debugln("MENU showing");
    } else if((menu_btn.getState() == Button::NotPressed) && (state == HOME)){
        display_default(ambient_temperature, set_point);
    }

    delay(10);

}