//
// Created by USER on 7/10/2023.
//

#ifndef SILVABACK_THERMOSTAT_DEFINES_H
#define SILVABACK_THERMOSTAT_DEFINES_H

#define BAUD_RATE 115200

// pins
#define RELAY 20
#define LOAD_ON 23 // indicate when the AC load is ON
#define BUZZER 18

uint32_t ENABLE_REMOTE =  1;

enum errors{
    PARAMETER_ERR = -1,
};

#define MAX_TEMPERATURE 60 // max temp in deg C
#define ROOM_TEMPERATURE 22 // set minimum to be at room temperature

// todo: ideally the reference temperature should be the room temperature


/**
 * Screen variables
 */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET  -1
#define SCREEN_ADDRESS 0x3C

enum states{
    HOME,
    MENU,
    SETTING
};

#define MENU_SIZE 4
uint32_t MENU_X_OFFSET = 15;
uint32_t MENU_Y_OFFSET = 17;

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
