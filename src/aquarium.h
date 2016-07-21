#pragma once

#include <Arduino.h>
#include <Automaton.h>
//#include <DS3232RTC.h>
#include <DS1307RTC.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Stepper.h>

namespace board {

    const struct used_pines_t {
        uint8_t digital_0;     // digital_0 = RX
        uint8_t button_yellow; // digital_1 = TX
        uint8_t button_blue;   // digital_2 - Switch lights
        uint8_t button_green;  // digital_3 - Switch autofill on and off
        uint8_t button_red;    // digital_4 - Switch filter
        uint8_t led_red;       // digital_5 - Filter is off
        uint8_t led_yellow;    // digital_6 - Autofill is off
        uint8_t led_blue;      // digital_7 - Filling
        uint8_t relay_lights;     // digital_8
        uint8_t relay_filter;  // digital_9
        uint8_t relay_unuser;    // digital_10
        uint8_t relay_pump;  // digital_11
        uint8_t buoy;          // digital_12
        uint8_t led_yellow2;   // digital_13

        uint8_t thermometer; // analog_0
        uint8_t stepper1; // analog_1
        uint8_t stepper2; // analog_2
        uint8_t stepper3; // analog_3
        uint8_t stepper4; // analog_4
        uint8_t analog_5; // analog_5

    } Pines = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
               10, 11, 12, 13, A0, A1, A2, A3, A4, A5};

    void setup();

    void loop();

    namespace auto_fill {
        void setup();
    }

    namespace filter {
        void setup();
    }

    namespace lights {
        void setup();

        void on();

        void off();

        const struct photo_period_t {
            int hour;
            int minute;
            OnTick_t func;
        } Photo_period[] = {{8,  0, lights::on},
                             {11, 0, lights::off},
                             {14, 30, lights::on},
                             {21, 0, lights::off}};
    }
/*
    namespace fans {
        void setup();

        void loop();

        const int too_hot = 30; // Temperature in celsius that trigger the fans
    }
*/
    namespace feeder {
        void setup();

        void feed();

        const struct feed_times_t {
            int hour;
            int minute;
            OnTick_t func;
        } Feed_times[] = {{ 8, 5,  feeder::feed},
                          {16, 58,  feeder::feed},
                          {20, 30, feeder::feed}
        };

        const timeDayOfWeek_t abstinence_day = dowSaturday;
    }
}
