//
// Created by USER on 7/11/2023.
//

#ifndef SILVABACK_THERMOSTAT_BUTTON_H
#define SILVABACK_THERMOSTAT_BUTTON_H

#include <Arduino.h>

class Button {
private:
    char _pin; // pin where the button is connected

    char _pressed_state; // pressed state can be HIGH or LOW
    static const int32_t DEBOUNCE_DELAY = 50;

    /* internal button state */
    enum InternalState{
        Idle,           // nothing happens
        DebounceDelay, // delaying
    };

    // the last time the btn was sampled
    char _last_time;


public:

    Button(uint32_t digital_pin_, uint32_t pressedState_);
    InternalState _internal_state;

    // possible button states
    enum ButtonState{
        NotPressed,
        Pressed
    };

    // get the current state of the button
    ButtonState getState();


};


#endif //SILVABACK_THERMOSTAT_BUTTON_H
