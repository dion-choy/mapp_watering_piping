#include "mbed.h"

DigitalOut relay(PA_0);  

void startPump() {
    printf("Starting pump...\n");
    
    printf("Pump active flag set\n");
    
    relay = 1;  // Turn on pump
    thread_sleep_for(2000);  // Run for 2 seconds
    relay = 0;  // Turn off pump
    
    thread_sleep_for(2000);  // Wait for water to settle
    printf("Pump cycle completed\n");
}