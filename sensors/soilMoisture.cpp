#include "mbed.h"

AnalogIn soilMoisture(PA_7);

float getMoist() {
    soilMoisture.set_reference_voltage(3.3f);

    return soilMoisture.read();
}