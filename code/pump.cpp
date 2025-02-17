#include "mbed.h"

DigitalOut relay(PA_0);  
extern bool pumpActive;  // Reference the global pumpActive flag

void startPump() {
    printf("Starting pump...\n");
    
    pumpActive = true;  // Set flag before starting pump
    printf("Pump active flag set\n");
    
    relay = 1;  // Turn on pump
    thread_sleep_for(2000);  // Run for 2 seconds
    relay = 0;  // Turn off pump
    
    thread_sleep_for(2000);  // Wait for water to settle
    pumpActive = false;  // Clear flag after pump stops
    printf("Pump cycle completed\n");
}
