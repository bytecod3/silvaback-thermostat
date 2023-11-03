//
// Created by Edwin on 7/10/2023.
//

#ifndef SILVABACK_THERMOSTAT_DEFINES_H
#define SILVABACK_THERMOSTAT_DEFINES_H

#define BAUD_RATE 115200

// pins
#define RELAY 33
#define LOAD_ON 18 // indicate when the AC load is ON
#define BUZZER 2

enum errors{
    PARAMETER_ERR = -1,
};

#define MAX_TEMPERATURE 100 // max temp in deg C
#define MIN_TEMPERATURE 0
#define ROOM_TEMPERATURE 25 // set minimum to be at room temperature

// todo: ideally the reference temperature should be the room temperature


/**
 * Screen variables
 */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET  (-1)
#define SCREEN_ADDRESS 0x3C
#define SPLASH_DELAY 3000 // show splash screen time
#define SLEEP_DELAY 3000 // wait for some seconds before sleeping the device, it feels better that way

enum states{
    HOME,
    MENU,
    ABOUT,
    CHANGE_UNITS,
    ENABLE_REMOTE,
    RESET,
    SET_TEMPERATURE,
    SLEEP
};

#define DEEP_SLEEP_COUNTDOWN 3000

uint32_t MENU_X_OFFSET = 15;
uint32_t MENU_Y_OFFSET = 17;

/*
Rotary encoder variables 
 */
#define SW 12
#define OUT_A 27
#define OUT_B 26

/*
 * Debug variables
 */
#define DEBUG 1

#if DEBUG == 1

#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#define debugf(x, y) Serial.printf(x, y)

#else

#define debug(x)
#define debugln(x)
#define debugf(x, y)

#endif


#endif //SILVABACK_THERMOSTAT_DEFINES_H
