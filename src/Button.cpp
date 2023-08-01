//
// Created by USER on 7/11/2023.
//

#include "Button.h"
#include <Arduino.h>

Button::Button(uint32_t digital_pin_, uint32_t pressedState_) {
    _pin = digital_pin_;
    _pressed_state = pressedState_;
    _internal_state = Idle;
    number_of_presses = 0;

    // set up the pin
    pinMode(digital_pin_, INPUT);
}

/*
 * get the current button state
 */
Button::ButtonState Button::getState() {
    uint32_t new_time;
    uint8_t state;

    // read the pin and toggle the state
    state = digitalRead(_pin);

    if(_pressed_state ==  LOW){
        state ^= HIGH;
    }

    if(state == LOW){
        _internal_state = Idle;
        return NotPressed;
    }

    // sample
    new_time = millis();

    switch (_internal_state) {
        case Idle:
            _internal_state = DebounceDelay;
            _last_time = new_time;
            break;

        case DebounceDelay:
            if(new_time - _last_time >= DEBOUNCE_DELAY){
                // the pin press is intentional
                return Pressed;
            }

            break;
    }

    return NotPressed;
}

