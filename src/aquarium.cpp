#include "aquarium.h"

namespace board {

	// http://playground.arduino.cc/Main/Printf
	void static p(const __FlashStringHelper *fmt, ...)
	{
    char buf[129]
    	PROGMEM;
    va_list args;
    va_start(args, fmt);
#ifdef __AVR__
    vsnprintf_P(buf, 128, (const char *) fmt, args);
#else
    vsnprintf(buf, 128, (const char *) fmt, args);
#endif
    va_end(args);
    Serial.print(buf);
	}

	void setup()
	{
    setSyncProvider(RTC.get);
    if (timeStatus() != timeSet)
      board::p(F("Unable to sync with the RTC"));

    board::p(F("Started at: %02d/%02d/%04d %2d:%02d:%02d\n"),
        day(), month(), year(), hour(), minute(), second());

		//    auto_fill::setup();
    filter::setup();
    lights::setup();
		//    fans::setup();
    feeder::setup();
	}

	void loop()
	{
    automaton.run();
    Alarm.delay(0);
		//        fans::loop();
	}
	/*
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
     buoy.begin(Pines.buoy, 2000, false, true);
     pump.begin(Pines.relay_pump, true);

     controller.begin()
     .IF(activated)
     .AND(buoy)
     .onChange(true, pump, pump.EVT_ON)
     .onChange(false, pump, pump.EVT_OFF)
     .led(Pines.led_blue);
//.trace(Serial)
}
}
*/
namespace filter { // and heater
	static Atm_button button;
	static Atm_led relay;
	static Atm_led led;
	static Atm_fan fan;

	void setup()
	{
    board::p(F("Filter - Warning led: %02d relay: %02d button: %02d\n"),
        Pines.led_red, Pines.relay_filter, Pines.button_red);

    led.begin(Pines.led_red);
    relay.begin(Pines.relay_filter, true);
    fan.begin().onInput(led, led.EVT_TOGGLE).onInput(relay, relay.EVT_TOGGLE);
    button.begin(Pines.button_red).onPress(fan, fan.EVT_INPUT);
    relay.trigger(relay.EVT_ON); // Relays acts upside down
	}
}

namespace lights {
	static Atm_button button;
	static Atm_led relay;

	void on()
	{
		//    board::p(F("Ligths ON\n"));
    relay.trigger(relay.EVT_ON); // Relays acts upside down
	}

	void off()
	{
		//    board::p(F("Lights OFF\n"));
    relay.trigger(relay.EVT_OFF);
	}

	static bool in_photo_period()
	{
    int H = hour();
    int M = minute();

    bool is_in = !((H >= Photo_period[0].hour && M >= Photo_period[0].minute &&
          H <= Photo_period[1].hour && M <= Photo_period[1].minute) ||
        (H >= Photo_period[2].hour && M >= Photo_period[2].minute &&
         H <= Photo_period[3].hour && M <= Photo_period[3].minute));

		//    board::p(F("Is In (%02d:%02d): %d\n"), H, M, is_in);

    return is_in;
	}

	void setup()
	{
    board::p(F("Lights - relay: %02d, button: %02d\n"),
        Pines.relay_lights, Pines.button_blue);
    relay.begin(Pines.relay_lights, true);
    button.begin(Pines.button_blue).onPress(relay, relay.EVT_TOGGLE);

    int n_periods = sizeof(Photo_period) / sizeof(struct photo_period_t);
    for (int i = 0; i < n_periods; i++) {
      board::p(F("Photoperiod %02d:%02d\n"),
          Photo_period[i].hour, Photo_period[i].minute);

      Alarm.alarmRepeat(Photo_period[i].hour, Photo_period[i].minute, 0,
          Photo_period[i].func);
    }

    if (in_photo_period())
      lights::on();
    else
      lights::off();
	}
}
/*
   namespace fans {
   OneWire oneWire(Pines.thermometer);
   DallasTemperature sensors(&oneWire);
   DeviceAddress outside_dev;

   Atm_led relay;
   boolean working;

   void start_fans(const uint8_t *device_address) {
   relay.trigger(relay.EVT_ON);
   working = true;
   }

   void setup() {
   board::p(F("Fans - relay: %02d, thermometer: %02d\n"), Pines.relay_fan, Pines.thermometer);

   relay.begin(Pines.relay_fan, true);
   working = false;

   sensors.begin();
   if (!sensors.getAddress(outside_dev, 1)) {
   board::p(F("Unable to find address for Device 1"));
   return;
   }
   sensors.setResolution(outside_dev, 9);
   sensors.setHighAlarmTemp(outside_dev, too_hot);
   sensors.setAlarmHandler(&start_fans);
   }

   void loop() {
   sensors.processAlarms();
   if (working) {
   if (!sensors.hasAlarm()) {
   relay.trigger(relay.EVT_OFF);
   working = false;
   }
   }
   }
   }
	 */
namespace feeder {
	static Atm_bit activated;
	static Atm_button button;

	const int steps_per_revolution = 32;
	const int revolutions = 1;
	Stepper stepper(steps_per_revolution, Pines.stepper1, Pines.stepper2, Pines.stepper3, Pines.stepper4);

	void change_status()
	{
    activated.trigger(activated.EVT_TOGGLE);
	}

	void feed()
	{
		//    if ((day() != abstinence_day) && (activated.ON)) {
    if (day() != abstinence_day) {
      board::p(F("Feeding the fishes"));
      stepper.step(-steps_per_revolution * 64 * revolutions);
    }
	}

	void setup()
	{
    stepper.setSpeed(150);

    int N = sizeof(Feed_times) / sizeof(struct feed_times_t);
    for (int i = 0; i < N; i++) {
      Alarm.alarmRepeat(Feed_times[i].hour, Feed_times[i].minute, 0,
          Feed_times[i].func);
      board::p(F("Feeding times %02d:%02d\n"),
          Feed_times[i].hour, Feed_times[i].minute);
    }

		//    activated.begin(true).led(Pines.led_yellow2, true);
		//    button.begin(Pines.button_yellow).onPress(activated, activated.EVT_TOGGLE);
	}
	}
}
