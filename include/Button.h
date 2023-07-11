//
// Created by USER on 7/11/2023.
//

#ifndef SILVABACK_THERMOSTAT_BUTTON_H
#define SILVABACK_THERMOSTAT_BUTTON_H


class Button {
private:
    char pin;

public:

    Button(char pin);
    char debounce_interval = 50;
    void pressed();
    void long_pressed();
    void debounce();

};


#endif //SILVABACK_THERMOSTAT_BUTTON_H
