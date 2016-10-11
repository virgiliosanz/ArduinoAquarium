#include "aquarium.h"

void setup()
{
    Serial.begin(9600);
    while (!Serial);

    board::setup();
}

void loop()
{
    board::loop();
}
