#pragma once

#include <Arduino.h>
#include <Automaton.h>
#include <DS3232RTC.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <Wire.h>

namespace board {

    const struct {
        uint8_t digital_0;    // digital_0 = RX
        uint8_t digital_1;    // digital_1 = TX
        uint8_t button_blue;  // digital_2 - Switch lights
        uint8_t button_green; // digital_3 - Switch autofill on and off
        uint8_t button_red;   // digital_4 - Switch filter
        uint8_t led_red;      // digital_5 - Filter is off
        uint8_t led_yellow;   // digital_6 - Autofill is off
        uint8_t digital_7;    // digital_7
        uint8_t relay_heater; // digital_8
        uint8_t relay_lights; // digital_9
        uint8_t relay_pump;   // digital_10
        uint8_t relay_filter; // digital_11
        uint8_t buoy;         // digital_12
        uint8_t led_blue;     // digital_13 - pump of autofill system is working

        uint8_t analog_0;    // analog_0
        uint8_t analog_1;    // analog_1
        uint8_t analog_2;    // analog_2
        uint8_t analog_3;    // analog_3
        uint8_t analog_4;    // analog_4
        uint8_t analog_5;    // analog_5
    } Pines
    PROGMEM = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
               10, 11, 12, 13, A0, A1, A2, A3, A4, A5};

    void setup();

    void loop();

    namespace auto_fill {
        void setup();
    }
    namespace filter {
        void setup();
    }
    namespace heater {
        void setup();
    }
    namespace lights {
        void setup();

        void on();

        void off();

        const struct {
            int hour;
            int minute;
            OnTick_t func;
        } Photo_period[4]
        PROGMEM = {{8,  0, lights::on},
                   {12, 0, lights::off},
                   {14, 0, lights::on},
                   {21, 0, lights::off}};
    }
}
