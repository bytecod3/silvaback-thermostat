#include <Arduino.h>
#include <driver/adc.h>
#include "defines.h"
#include "Temperature.h"
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

/**
 * Configure ADC_RTC
 */
void config_adc_rtc(){
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
}

uint32_t read_divider_analog_value(){
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

/**
 * Buzzer sound
 */
void buzz(){
    digitalWrite(BUZZER, HIGH);
    delay(200);
    digitalWrite(BUZZER, LOW);
}


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
    display.println(val);
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
unsigned long debounce_delay = 0;

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

        if(counter < 4){
            // set boundary condition for menu - last item in menu todo:change 4 to menu size
            counter++; //increase counter
        }

        encoder_value = 0;

    } else if(encoder_value < -2){ // two steps backwards
        encoder_direction = "CCW";
        if(counter > 0){
            // set boundary condition - first item of menu
            counter--;
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

uint32_t menu_item = 0;
uint32_t frame = 1;
uint32_t page = 1;
uint32_t last_menu_item = 1;
uint32_t menu_timeout = 6000; // if menu goes inactive for this state, return to home state
#define MENU_SIZE 5

bool up = false;
bool down = false;
bool middle = false;
bool menu_state = false; // true if we get into displaying the menu

String menu_items[MENU_SIZE] = {
        "Set temperature",
        "Enable remote",
        "Sleep",
        "Change units",
        "Reset"
};

// enable remote options
String enable_remote[2] = {
    "Yes",
    "No"
};
uint32_t selected_remote_state = 0;

String change_units[2] = {
        "C",
        "F"
};
uint32_t selected_units = 0;

void drawMenu(uint32_t item){
    /**
     * display static menu
     */
    display.clearDisplay();
    display.setCursor(15, 0);
    display.println("Menu");

    /*========================== */
    switch(item){
        case(0):
            display.setCursor(0, MENU_Y_OFFSET);
            display.println(">");
            break;

        case (1):
            display.setCursor(0, MENU_Y_OFFSET + 10);
            display.println(">");
            break;

        case (2):
            display.setCursor(0, MENU_Y_OFFSET + 20);
            display.println(">");
            break;

        case (3):
            display.setCursor(0, MENU_Y_OFFSET + 30);
            display.println(">");
            break;

        case (4):
            display.setCursor(0, MENU_Y_OFFSET + 40);
            display.println(">");
            break;

        default:

            display.setCursor(0, MENU_Y_OFFSET);
            display.println(">");
            break;
    }

    for (int i = 0; i < MENU_SIZE; ++i) {
        display.setCursor(MENU_X_OFFSET, MENU_Y_OFFSET);
        display.println(menu_items[i]);

        MENU_Y_OFFSET += 10;

    }

    MENU_Y_OFFSET = 17;

    display.display();

}

// set temperature
void setTemperature(){
    display.clearDisplay();
    display.setCursor(20, 0);
    display.println("Set temperature");

    display.setCursor(40, 40);
    display.setFont(&FreeMonoBold18pt7b);

    display.println(set_point);

    display.setFont();
    display.display();

}


/*
 * ======================================= END OF MENU FUNCTIONS =============================
 */


/**
 * Switch ON or OFF the HVAC voltage switch - relay
 */
void activate_HVAC(uint32_t set, uint32_t ambient){
    // if ambient temperature is less than the set point, warm the room
    // if the ambient temperature is more than the set point, it is hot, cool the room

    if(ambient > set){
        digitalWrite(RELAY, HIGH);

        // visual report on the LOAD_ON led
        digitalWrite(LOAD_ON, HIGH);
    } else {
        digitalWrite(RELAY, LOW);
        digitalWrite(LOAD_ON, LOW);
    }
}

/*
 * Allow the user to set the reference temperature
 * called when the user chooses set temperature from the menu
 */
//void set_reference_temperature(){
//    // poll the increment and decrement buttons
//    if(up_btn.getState() == Button::Pressed){
//        set_point++;
//
//        // check against the threshold temperature
//        if(set_point >= MAX_TEMPERATURE){
//            buzz();
//
//            // todo: lock the temperature from incrementing on the screen
//
//        }
//    }
//
//    if(down_btn.getState() == Button::Pressed){
//        set_point--;
//
//        if(set_point <= ROOM_TEMPERATURE){
//            buzz();
//
//            // todo: lock the temperature from going below the room_temperature
//        }
//    }
//}

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

void setup() {
    Serial.begin(BAUD_RATE);

    // configure ADC
    config_adc_rtc();

    // configure screen
    config_screen();

    // set default mode
    display_default(ambient_temperature, set_point);

    // set rotary encoder pin-outs
    encoderInterruptAttach();

    // initial operating state is home
    state = HOME;
}

void loop() {

    analog_out = read_divider_analog_value();
    ambient_temperature = calculate_temp_in_deg_C(analog_out);

    if (ambient_temperature == PARAMETER_ERR){
        debugln("Err:Could not read");
    }

    display_default(ambient_temperature, set_point);

    /*
     * Handle encoder rotation
     */
    static uint8_t last_counter = 0;

    /*
     * Handle encoder button press
     */

    if(encoder_button.pressed && state == states::HOME){
        // debug("Button pressed "); debug(encoder_button.no_of_presses);debugln();

        state = states::MENU; // change operating state to menu
        encoder_button.pressed = false;

        /*
         * =============== DRAW MENU =======================================
         */
        drawMenu(menu_item);
        do {

            // debugln(encoder_direction);

            if(encoder_direction == "CCW"){
                debug("Up"); debugln(counter);

                drawMenu(counter);

                // if encoder button is clicked while the menu is being displayed,
                // call the corresponding menu function
                if(encoder_button.pressed){
                    if(counter == 0){
                        state = states::MENU_ITEM_ONE;
                        debugln(menu_items[counter]);

                        state = states::MENU_ITEM_ONE;
                        do{
                            setTemperature();
                            state = states::MENU_ITEM_ONE;
                        } while(state);

                        debugln(state);
                    }
                }

                encoder_button.pressed = false;

            } else if(encoder_direction == "CW"){
                debug("Down"); debugln(counter);

                drawMenu(counter);

                debugln(menu_items[counter]);

            }

        } while(state == MENU);

        // if button is pressed here,

        if(encoder_button.pressed && state == MENU){
            display_default(ambient_temperature, set_point);
        }


    }

    // reset state to home
    state = HOME;


}
