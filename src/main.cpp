#include <Arduino.h>
#include <driver/adc.h>
#include "defines.h"
#include "Temperature.h"
#include "splash_screen.h"
#include <SPI.h>
#include <Preferences.h>
#include <U8g2lib.h>

// surrounding temperature
uint32_t ambient_temperature = 0;
uint32_t set_point = ROOM_TEMPERATURE;
void setTemperature();

// initialize NVS for storing user set temperature
Preferences user_data;

// default mode = HOME
uint8_t state = HOME;

/**
 * Configure ADC_RTC
 */
void config_adc_rtc(){
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11); // pin 36  
}

uint32_t read_divider_analog_value(){
    uint32_t val = adc1_get_raw(ADC1_CHANNEL_0);
    return val;
}

/**
 * Configure the screen
 */
#define SCREEN_HEIGHT 64
#define SCREEN_WIDTH 128
#define YELLOW_OFFSET 16
#define OLED_SDA 21
#define OLED_SCL 22
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(/*R2: rotation 180*/U8G2_R0, /*reset*/U8X8_PIN_NONE, /* clock */ OLED_SCL, /* data */ OLED_SDA);

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

/**
 * Buzzer sound
 */
void buzz(){
    digitalWrite(BUZZER, HIGH);
    delay(200);
    digitalWrite(BUZZER, LOW);
    delay(200);
}

void display_default(uint32_t val){
  u8g2.clearDisplay();
  u8g2.clearBuffer();
  u8g2.print(val);

  u8g2.sendBuffer();

}

/**
* ====================================== ROTARY ENCODER =============================
*/
struct Button{
    const uint32_t pin;
    uint32_t no_of_presses;
    bool pressed;
};

uint8_t encoder_pinA =  14;
uint8_t encoder_pinB = 27;
uint32_t encoder_button_pin = 12;
uint32_t no_of_presses = 0;
String encoder_direction = "";

Button encoder_button = {encoder_button_pin, 0, false}; // todo: debounce this with schmitt

volatile uint32_t counter = 0;
unsigned long debounce_delay = 50;
uint32_t previous_time = 0;
uint32_t current_time = 0;

/*
 * Interrupt service routine for handling encoder button rotation
 */
void IRAM_ATTR readEncoder(){
    static uint8_t old_ab = 3;
    static int8_t encoder_value = 0; // encoder value
    static const int enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};

    old_ab <<= 2; // remember previous state

    if(digitalRead(encoder_pinA)) old_ab |= 0x02; // add current state of pin A

    if(digitalRead(encoder_pinB)) old_ab |= 0x01; // add current state of pin B

    encoder_value += enc_states[(old_ab & 0x0f)];

    if(encoder_value > 2){ // two steps forward
        encoder_direction = "CW";

        if(state == states::MENU) {
            if(counter < 4){
                // set boundary condition for menu - last item in menu todo:change 4 to menu size
                counter++; //increase counter
            }
        } else if(state == states::MENU_ITEM_ONE){
            // set boundary conditions for setting temperature
            if(counter < 70){
                counter++; // cannot go above 70 deg todo: beep here
            }
        }

        encoder_value = 0;

    } else if(encoder_value < -2){ // two steps backwards
        encoder_direction = "CCW";

        if(state == states::MENU){
            if(counter > 0){
                // set boundary condition - first item of menu
                counter--;
            }
        } else if (state == states::MENU_ITEM_ONE){
            if (counter > 0){
                counter--;

                if (counter < 0){
                    // cannot go below zero
                    counter = 0;
                }

                // todo: beep here
            }
        }

        encoder_value = 0;
    }
}

/*
 * Interrupt service routine for encoder button press
 */
void IRAM_ATTR encoderButtonPress(){
    encoder_button.no_of_presses++;
    encoder_button.pressed = true;

}

/*
 * =======================================FEATURE FUNCTIONS=================================
 */


/*
 * =============================END OF FEATURE FUNCTIONS===================================
 */


/*
 * Attach interrupts for encoder pins
 */
void encoderInterruptAttach(){
    pinMode(encoder_pinA, INPUT_PULLUP);
    pinMode(encoder_pinB, INPUT_PULLUP);
    pinMode(encoder_button_pin, INPUT_PULLUP);

    attachInterrupt(encoder_pinA, readEncoder, CHANGE);
    attachInterrupt(encoder_pinB, readEncoder, CHANGE);
    attachInterrupt(encoder_button.pin, encoderButtonPress, FALLING);

}

/**
 * ====================== END OF ROTARY ENCODER FUNCTIONS ====================================
 */

/*
 * ========================================== MENU ================================
 */

bool up = false;
bool down = false;
bool middle = false;
bool menu_state = false; // true if we get into displaying the menu

// ' icon_about', 16x16px
const unsigned char epd_bitmap__icon_about [] PROGMEM = {
  0x00, 0x00, 0x07, 0xc0, 0x08, 0x20, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x20, 0x08, 0x40, 
  0x04, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00
};
// ' icon_change_units', 16x16px
const unsigned char epd_bitmap__icon_change_units [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x1f, 0xe0, 0x20, 0x10, 0x40, 0x08, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 
  0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x08, 0x20, 0x10, 0x1f, 0xe0, 0x00, 0x00
};
// ' icon_enable_remote', 16x16px
const unsigned char epd_bitmap__icon_enable_remote [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x3f, 0xfe, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x3f, 0xf2, 0x00, 0x12, 
  0x00, 0x12, 0x3f, 0x12, 0x01, 0x12, 0x01, 0x12, 0x39, 0x12, 0x09, 0x12, 0x09, 0x12, 0x00, 0x00
};
// ' icon_reset', 16x16px
const unsigned char epd_bitmap__icon_reset [] PROGMEM = {
  0x00, 0x00, 0x07, 0xc0, 0x08, 0x30, 0x10, 0x08, 0x20, 0x04, 0x40, 0x04, 0x40, 0x02, 0x40, 0x02, 
  0x40, 0x02, 0x20, 0x84, 0x30, 0x84, 0x10, 0x84, 0x1c, 0x88, 0x02, 0x90, 0x01, 0x80, 0x3f, 0x80
};
// ' icon_set_temp', 16x16px
const unsigned char epd_bitmap__icon_set_temp [] PROGMEM = {
  0x00, 0x00, 0x0e, 0x38, 0x11, 0x28, 0x11, 0x38, 0x19, 0x00, 0x11, 0x00, 0x19, 0x00, 0x11, 0x00, 
  0x19, 0x00, 0x11, 0x00, 0x28, 0x80, 0x48, 0x40, 0x7f, 0xc0, 0x7f, 0xc0, 0x3f, 0x80, 0x00, 0x00
};
// ' icon_sleep', 16x16px
const unsigned char epd_bitmap__icon_sleep [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x02, 0x00, 0x04, 0x70, 0x08, 0x20, 
  0x10, 0x46, 0x3f, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 288)
const unsigned char* bitmap_icons[6] = {
  epd_bitmap__icon_about,
  epd_bitmap__icon_change_units,
  epd_bitmap__icon_enable_remote,
  epd_bitmap__icon_reset,
  epd_bitmap__icon_set_temp,
  epd_bitmap__icon_sleep
};
// ' item_sel_background', 125x21px
const unsigned char epd_bitmap__item_sel_background [] PROGMEM = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe8, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe8, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08
};
// ' scrollbar_background', 4x64px
const unsigned char epd_bitmap__scrollbar_background [] PROGMEM = {
  0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x00, 
  0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 
  0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 
  0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x20, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00
};


/*
 * Show splash screen when device first boots
 */
void showSplashScreen(){
    u8g2.firstPage();
    do{
        u8g2.drawBitmap(0, 0, 128/8, 64, epd_bitmap_splash_screen);
    } while(u8g2.nextPage());
    
}

const int NUM_ITEMS = 6;

char menu_items[NUM_ITEMS][20] = {
  {"About"},
  {"Change units"},
  {"Enable remote"},
  {"Reset"},
  {"Set temperature"},
  {"Sleep"}
};

int selected_item = 0; // which item in the menu is selected
int previous_item; // item before the selected one
int next_item; // item after the selected one

/*
 * ======================================= END OF MENU FUNCTIONS =============================
 */


/**
 * Switch ON or OFF the HVAC voltage switch - relay
 */
void activate_HVAC(uint32_t set, uint32_t ambient){
    // if ambient temperature is less than the set point, turn on heating element
    // if the ambient temperature is more than the set point, it is hot, turn off heating element

    debugln(ambient);
    debugln(set);
    if(ambient > set){
        digitalWrite(RELAY, LOW); // turn off heating element

        // visual report on the LOAD_ON led
        digitalWrite(LOAD_ON, LOW);
    } else if( ambient < set) {
        digitalWrite(RELAY, HIGH); // turn on heating element
        digitalWrite(LOAD_ON, HIGH);
    } else{
        // if equal, no change
        digitalWrite(RELAY, LOW);
        digitalWrite(LOAD_ON, LOW);
    }
}

/**
 * Activate WIFI subsystem
 */
void init_WIFI(){

}

/**
 * Deactivate WIFI subsystem
 */
void deactivate_WIFI(){

}

/**
 * Enable WIFI control of the system
 */
//void enable_remote(){
//    // poll the buttons to check if enable_remote flag is set
//    if(ENABLE_REMOTE){
//        // activate WI-FI sub-system
//        init_WIFI();
//    } else{
//        // deactivate WI-FI sub-system
//        deactivate_WIFI();
//    }
//}

void wakeup(){
    esp_sleep_wakeup_cause_t wake_up_source;

    wake_up_source = esp_sleep_get_wakeup_cause();

    switch(wake_up_source){
        case ESP_SLEEP_WAKEUP_EXT0 : debugln("Wake-up from external signal with RTC_IO"); break;
        case ESP_SLEEP_WAKEUP_EXT1 : debugln("Wake-up from external signal with RTC_CNTL"); break;
        case ESP_SLEEP_WAKEUP_TIMER : debugln("Wake up caused by a timer"); break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD : debugln("Wake up caused by a touchpad"); break;
        default : Serial.printf("Wake up not caused by Deep Sleep: %d\n",wake_up_source); break;
    }

    ESP.restart();
}

int encoder_clockwise = 0;
int encoder_counterclockwise = 0;
String current_encoder_direction;
String previous_encoder_direction;
unsigned long encoder_unclick_current_time = 0;
unsigned long encoder_unclick_previous_time = 0;
unsigned long interval = 500;

void setup() {
    Serial.begin(BAUD_RATE);

    pinMode(LOAD_ON, OUTPUT);
    pinMode(RELAY, OUTPUT);

    // configure ADC
    config_adc_rtc();

    // configure screen
    u8g2.setColorIndex(1);  // set the color to white
    u8g2.begin();

    // set default mode
    showSplashScreen();
    // delay(SPLASH_DELAY);

    // set rotary encoder pin-outs
    encoderInterruptAttach();

    // initial operating state is home
    state = HOME;

    // initialize user config memory
    user_data.begin("config_data", false);

    // save set_point to memory
    user_data.putUInt("set_point", set_point);

    // get last saved set point from memory
    set_point = user_data.getUInt("config_data", ROOM_TEMPERATURE);

    debugln("Created NVS partition");

//    wakeup();

    // enable external RTC GPIO wakeup
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_25, HIGH);

}

void loop() {

    analog_out = read_divider_analog_value();
    ambient_temperature = calculate_temp_in_deg_C(analog_out);

    if (ambient_temperature == PARAMETER_ERR){
        debugln("Err:Could not read"); // log this efficiently - create a logger class
    }

    // if encoder button is pressed initially, change state to menu
    if(encoder_button.pressed && state == states::HOME){
        encoder_button.pressed = false; // reset the encoder button
        state = states::MENU;
    }

    while(state == states::MENU){

        if(encoder_direction == "CCW"){
            // jump to the previous menu item
            selected_item = selected_item - 1; // select the previous item
            encoder_counterclockwise = 1; // ensure item is only selected once
        
        
            if(selected_item < 0){
                // if first item is selected, jump to the last item
                selected_item = NUM_ITEMS - 1;
            }

        } else if(encoder_direction == "CW"){

            // jump to the next menu item 
            selected_item = selected_item + 1;
            encoder_clockwise = 1; // ensure item is only selected once

            if(selected_item >= NUM_ITEMS){ // if last item was selected, jump to the first item in the list
                selected_item = 0;
            }

        }

        encoder_direction = "";

        // handle encoder button press - while we are in the menu state

        // calculate the menu items
        previous_item = selected_item - 1;
        if(previous_item < 0){
            previous_item = NUM_ITEMS - 1; // wrap to the last menu item
        } 

        next_item = selected_item + 1;
        if(next_item >= NUM_ITEMS){
            next_item = 0; // wrap to the first menu item
        }

        // render items on the screen 
        u8g2.firstPage();
        do {
            
            if(state == states::MENU){ // 
                // selected item background
                u8g2.drawBitmap(0, 22, 128/8, 21, epd_bitmap__item_sel_background);

                // previous item
                //u8g2.setFont(u8g2_font_ncenB14_tr);
                u8g2.setFont(u8g2_font_7x14_tr);
                u8g2.drawBitmap(3, 2, 16/8, 16, bitmap_icons[previous_item]);
                u8g2.drawStr(24,15,menu_items[previous_item]);

                // selected item
                u8g2.setFont(u8g2_font_7x14_tr);
                u8g2.drawBitmap(3, 24, 16/8, 16, bitmap_icons[selected_item]);
                u8g2.drawStr(24,38,menu_items[selected_item]);

                // next item
                u8g2.setFont(u8g2_font_7x14_tr);
                u8g2.drawBitmap(3, 47, 16/8, 16, bitmap_icons[next_item]);
                u8g2.drawStr(24,59, menu_items[next_item]);
                
                // scrollbar
                u8g2.drawBitmap(124, 0, 4/8, 64, epd_bitmap__scrollbar_background);
            }

            
        } while ( u8g2.nextPage() ); 

      
    }

    // reset the state to home for the next iteration
    state = states::HOME;


    /*
     * =========================== CONTROL HEATING ELEMENT =================================
     */
    //activate_HVAC(set_point, ambient_temperature);

    /*
     * =========================== END OF HEATING ELEMENT CONTROL ===========================
     */


}
