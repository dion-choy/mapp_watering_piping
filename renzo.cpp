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

// Our header files
#include "lcd.h"    
#include "check_water.hpp"  // For getDist() and ultrasonic sensor
#include "keypad.hpp"      // For displayMessage() and other keypad functions

// Remove duplicate variables that are in keypad.cpp
// unsigned char key, outChar;

// Message strings for leak detection
const char Message[] = "Normal";    
const char Message1[] = "Warning! Water is";	
const char Message2[] = "leaking!";  

// Leak detection variables
float objDist = 0;
bool waterFlag = true;
bool isInitialized = false;

// Keep these constants
const float LEAK_THRESHOLD = 0.5; // cm 
const int MEASUREMENT_DELAY = 1000; // ms
const int CONFIRMATION_READINGS = 2; // no. of readings to confirm leak

// Remove these as they're already in check_water.cpp
// DigitalOut Trig(PC_5);
// InterruptIn Echo(PC_6);
// Timer echoTimer;
// Timeout echoTimeOutTimer;

// Add at the top with other global variables
float currentDist = 0;  // Make this global
Ticker distanceUpdate;  // Add Ticker for continuous updates

// Add this function to update distance
void updateDistance() {
    currentDist = getDist();
    printf("Current Distance: %.2f cm\n", currentDist);
}

// ---- Main Program ---------------------------------------------------------------
bool leak_detect()
{
    // Remove this print since it's not needed on every check
    // printf("This is MAPP LCDKeypad running on Mbed OS %d.%d.%d.\n", ...);

    // Initialize reference distance on first call
    if (!isInitialized) {
        objDist = getDist();
        isInitialized = true;
        printf("Reference distance: %.2f cm\n", objDist);
        return false;
    }
    
    // Don't clear display here - moved to main loop
    // lcd_write_cmd(0x01); 
    
    // Check if water level has increased
    if (currentDist < objDist) {
        objDist = currentDist;
        printf("Water level increased. New reference distance: %.2f cm\n", objDist);
    }
    
    if (!waterFlag && currentDist > (objDist + LEAK_THRESHOLD)) {
        lcd_write_cmd(0x01); // Clear only when leak detected
        lcd_write_cmd(0x80); 
        displayMessage(Message1); 
        lcd_write_cmd(0xC0);     
        displayMessage(Message2);
        printf("Leak detected! Current: %.2f cm, Reference: %.2f cm\n", currentDist, objDist);
        return true;
    }
    
    return false;
}

int main()
{
    printf("Leak Detection System Starting...\n");
    
    // Initialize LCD
    lcd_init();
    
    // Start continuous distance updates every 1.5 seconds
    distanceUpdate.attach(&updateDistance, 1.5);
    
    // Initial display setup
    lcd_write_cmd(0x01);
    lcd_write_cmd(0x80);
    displayMessage("Press 1:Flow 2:Stop");
    
    while(1) {
        // Get user input
        char input = getkey();
        
        // Clear display only on input
        lcd_write_cmd(0x01);
        lcd_write_cmd(0x80);
        
        // Update water flow state
        if(input == '1') {
            waterFlag = true;
            displayMessage("Water flow ON     ");
            printf("Water flow enabled\n");
        }
        else if(input == '2') {
            waterFlag = false;
            displayMessage("Water flow OFF    ");
            printf("Water flow disabled\n");
        }
        
        // Show status on second line
        lcd_write_cmd(0xC0);
        if(!leak_detect()) {
            displayMessage("Status: Normal    ");
        }
        
        thread_sleep_for(500); // Reduced delay
    }
}
