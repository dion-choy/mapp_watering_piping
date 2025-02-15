#include "mbed.h"

#define DARK 0.7f
#define DIM 0.4f

AnalogIn LDRAnalog(PA_6);

int getBright() {
    LDRAnalog.set_reference_voltage(3.3f);

    if (LDRAnalog.read() > DARK) {
        return 0;   // dark
    } else if (LDRAnalog.read() > DIM) {
        return 1;   // dim
    } else {
        return 2;   // bright
    }
}