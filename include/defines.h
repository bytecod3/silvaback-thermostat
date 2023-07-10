//
// Created by USER on 7/10/2023.
//

#ifndef SILVABACK_THERMOSTAT_DEFINES_H
#define SILVABACK_THERMOSTAT_DEFINES_H

#define BAUD_RATE 115200

enum errors{
    PARAMETER_ERR = -1,
};

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
