// LCDKeypad.cpp
// Program to test LCD.
// The LCD display with two lines, 20 characters each.
// PORT A1: PA_15 : PA_8
// There are three control lines (PA_14:PA_12) and four data lines (PA_11 : PA_8).
// PA_14 - RS=0 Data represents Command, RS=1 Data represents Character
// PA_13 - RW=0 Writing into the LCD module
// PA_12 - E =1 Data is latched into LCD module during low to hight transition  
#undef __ARM_FP
#undef __ARM_use_no_argv

// System includes
#include "mbed.h"
#include <chrono>
using namespace std::chrono;

// Our header files
#include "lcd.h"    
#include "keypad.h"
#include "lcdscroll.hpp"
#include "sensors/sensors.hpp"  // For getDist()

// Remove duplicate variables that are in keypad.cpp
// unsigned char key, outChar;

// Message strings for leak detection
const char Message[] = "Normal";    
const char Message1[] = "Warning!          ";	
const char Message2[] = "Leak Detected!    ";  // Adjusted to match LCD width

// Leak detection variables
float objDist = 0;
bool isInitialized = false;

// Constants
const float LEAK_THRESHOLD = 0.5; // cm 
const int MEASUREMENT_DELAY = 1000; // ms

// Global variables
float currentDist = 0;
Ticker distanceUpdate;

// Add this function to update distance
void updateDistance() {
    currentDist = getDist();
    printf("Current Distance: %.2f cm\n", currentDist);
}

// ---- Main Program ---------------------------------------------------------------
bool leak_detect()
{
    if (!isInitialized) {
        objDist = getDist();
        isInitialized = true;
        printf("Reference distance: %.2f cm\n", objDist);
        return false;
    }
    
    if (currentDist < objDist) {
        objDist = currentDist;
        printf("Water level increased. New reference: %.2f cm\n", objDist);
    }
    
    if (!waterFlag && currentDist > (objDist + LEAK_THRESHOLD)) {
        // Clear current display and show leak warning
        lcd_write_cmd(0x01);
        lcd_write_cmd(0x80); 
        displayMessage(Message1);
        lcd_write_cmd(0xC0);     
        displayMessage(Message2);
        printf("Leak detected! Current: %.2f cm, Reference: %.2f cm\n", currentDist, objDist);
        thread_sleep_for(2000);  // Show leak message for 2 seconds
        update_display(true);    // Return to menu
        return true;
    }
    
    return false;
}

int main()
{
    printf("Leak Detection System Starting...\n");
    
    // Initialize LCD
    lcd_init();
    
    // Start continuous distance updates
    distanceUpdate.attach(&updateDistance, 1000ms);
    
    // Initial display setup
    update_display(true);
    
    while(1) {
        char key = getkey();
        
        switch(key) {
            case '2': // Up
                scroll_up();
                break;
            case '8': // Down
                scroll_down();
                break;
            case '5': // Select
                select_option();
                break;
        }
        
        // Check for leaks
        leak_detect();
        
        thread_sleep_for(1500);
    }
}
