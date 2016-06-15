#include "aquarium.h"
#include <avr/pgmspace.h>
#include <stdarg.h>

namespace board {

// http://playground.arduino.cc/Main/Printf
    void static p(const __FlashStringHelper *fmt, ...) {
        char buf[129]
        PROGMEM;
        va_list args;
        va_start(args, fmt);
#ifdef __AVR__
        vsnprintf_P(buf, 128, (const char *)fmt, args);
#else
        vsnprintf(buf, 128, (const char *) fmt, args);
#endif
        va_end(args);
        Serial.print(buf);
    }

    void setup() {
        setSyncProvider(RTC.get);
        if (timeStatus() != timeSet)
            board::p(F("Unable to sync with the RTC"));

        board::p(F("Started at: %02d/%02d/%04d %2d:%02d:%02d\n"), day(), month(),
                 year(), hour(), minute(), second());

        auto_fill::setup();
        filter::setup();
        heater::setup();
        lights::setup();
    }

    void loop() {
        automaton.run();
        Alarm.delay(0);
    }

    namespace auto_fill {
        static Atm_button button;
        static Atm_digital buoy;
        static Atm_led pump;
        static Atm_bit activated;
        static Atm_controller controller;

        void setup() {
            board::p(F("Auto_Fill - Working led: %02d Active led: %02d button: %02d pump "
                               "%02d buoy: %02d\n"),
                     Pines.led_yellow, Pines.led_blue, Pines.button_green,
                     Pines.relay_pump, Pines.buoy);

            activated.begin(true).led(Pines.led_yellow, true);

            button.begin(Pines.button_green).onPress(activated, activated.EVT_TOGGLE);
            //.trace(Serial);

            buoy.begin(Pines.buoy, 100, false, true);
            //.trace(Serial);

            pump.begin(Pines.relay_pump);
            //.trace(Serial);

            controller.begin()
                    .IF(activated)
                    .AND(buoy)
                    .onChange(true, pump, pump.EVT_OFF)
                    .onChange(false, pump, pump.EVT_ON)
                    .led(Pines.led_blue);
            //.trace(Serial)
        }
    }

    namespace filter {
        static Atm_button button;
        static Atm_led relay;
        static Atm_led led;
        static Atm_fan fan;

        void setup() {
            board::p(F("Filter - Warning led: %02d relay: %02d button: %02d\n"),
                     Pines.led_red, Pines.relay_filter, Pines.button_red);

            led.begin(Pines.led_red);
            relay.begin(Pines.relay_filter);
            //.trace(Serial);

            fan.begin().onInput(led, led.EVT_TOGGLE).onInput(relay, relay.EVT_TOGGLE);

            button.begin(Pines.button_red).onPress(fan, fan.EVT_INPUT);
            //.trace(Serial);

            relay.trigger(relay.EVT_OFF); // Relays acts upside down
        }
    }
    namespace heater {
        static Atm_led relay;

        void setup() {
            board::p(F("Heater - relay: %02d\n"), Pines.relay_heater);

            relay.begin(Pines.relay_heater);
            //.trace(Serial);
            relay.trigger(relay.EVT_OFF);
        }
    }

    namespace lights {
        static Atm_button button;
        static Atm_led relay;

        void on() {
            board::p(F("Ligths ON\n"));
            relay.trigger(relay.EVT_OFF); // Relays acts upside down
        }

        void off() {
            board::p(F("Lights OFF\n"));
            relay.trigger(relay.EVT_ON);
        }

        static bool in_photo_period() {
            int H = hour();
            int M = minute();

            bool is_in = !((H >= Photo_period[0].hour && M >= Photo_period[0].minute &&
                     H <= Photo_period[1].hour && M <= Photo_period[1].minute) ||
                    (H >= Photo_period[2].hour && M >= Photo_period[2].minute &&
                     H <= Photo_period[3].hour && M <= Photo_period[3].minute));

           // board::p(F("Is In: %d\n"), is_in);

            return is_in;
        }

        void setup() {
            board::p(F("Lights - relay: %02d, button: %02d\n"), Pines.relay_lights,
                     Pines.button_blue);
           board::p(F("Photoperiod 1 %02d:%02d -> %02d:%02d\n"),
              Photo_period[0].hour,Photo_period[0].minute,
              Photo_period[1].hour,Photo_period[1].minute);
            board::p(F("Photoperiod 2 %02d:%02d -> %02d:%02d\n"),
              Photo_period[2].hour,Photo_period[2].minute,
              Photo_period[3].hour,Photo_period[3].minute);

            relay.begin(Pines.relay_lights);
            //.trace(Serial);

            button.begin(Pines.button_blue).onPress(relay, relay.EVT_TOGGLE);
            //.trace(Serial);

            for (int i = 0; i < 4; i++) {
                Alarm.alarmRepeat(Photo_period[i].hour, Photo_period[i].minute, 0,
                                  Photo_period[i].func);
            }

            if (in_photo_period())
              lights::on();
            else
              lights::off();
        }
    }
}
