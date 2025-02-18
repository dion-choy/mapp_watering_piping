#include "mbed.h"

DigitalOut buzzer(PB_12);

void startBuzzer(void) //Function to generate one tone
{
    unsigned int k;
    for (k = 0; k < 100; k++)
    {
        wait_us(3000); //
        buzzer = !buzzer; //Invert logic level at PA_1
    }
}