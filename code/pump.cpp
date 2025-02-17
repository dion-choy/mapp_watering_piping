#include "mbed.h"
#include "exports.hpp"

DigitalOut relay(PA_0);  
void initPump(){
    relay = 1;
}
void startPump() {
    printf("Starting pump...\n");
    
    pumpRunning = true;  // Set flag before starting pump
    printf("Pump active flag set\n");
    
    relay = 0;  // Turn on pump
    thread_sleep_for(2000);  // Run for 2 seconds
    relay = 1;  // Turn off pump
    
    thread_sleep_for(2000);  // Wait for water to settle
    pumpRunning = false;  // Clear flag after pump stops
    printf("Pump cycle completed\n");
}
