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
const char Message[] = "Normal            ";    
const char Message1[] = "Warning!          ";	
const char Message2[] = "Leak Detected!    ";

// Leak detection variables
float objDist = 0;
bool isInitialized = false;

// Constants
const float LEAK_THRESHOLD = 1.0; // cm
const int MEASUREMENT_DELAY = 1000; // ms

unsigned char outChar;

// Global variables
float currentDist = 0;
Ticker distanceUpdate;

void updateDistance() {
    currentDist = getDist();
    if (currentDist > 0) {  // Only print valid readings
        printf("[SENSOR] Distance: %.2f cm, Water: %s\n", 
               currentDist, 
               waterFlag ? "ON" : "OFF");
    }
}

bool leak_detect()
{
    if (!isInitialized) {
        objDist = getDist();
        if (objDist <= 0) return false;  // Invalid reading
        isInitialized = true;
        printf("[INIT] Reference distance set to: %.2f cm\n", objDist);
        return false;
    }
    
    if (currentDist <= 0) return false;  // Skip invalid readings
    
    if (currentDist < objDist) {
        printf("[LEVEL] Water level increased: %.2f -> %.2f cm\n", objDist, currentDist);
        objDist = currentDist;
    }
    
    if (!waterFlag && currentDist > (objDist + LEAK_THRESHOLD)) {
        printf("[LEAK] Detected! Current: %.2f cm, Ref: %.2f cm\n", currentDist, objDist);
        
        // Display leak warning
        lcd_write_cmd(0x01);
        ThisThread::sleep_for(50ms);  // Use ThisThread instead of thread_sleep_for
        
        lcd_write_cmd(0x80);
        for (int i = 0; i < 20; i++) {
            outChar = Message1[i];
            lcd_write_data(outChar);
        }
        
        lcd_write_cmd(0xC0);
        for (int i = 0; i < 20; i++) {
            outChar = Message2[i];  // Changed from Message1 to Message2
            lcd_write_data(outChar);
        }
        
        ThisThread::sleep_for(2s);  // Use ThisThread and chrono literals
        update_display(true);
        return true;
    }
    
    return false;
}

int main()
{
    printf("\n[START] Leak Detection System Starting...\n");
    printf("[CONFIG] Threshold: %.2f cm\n", LEAK_THRESHOLD);
    
    lcd_init();
    ThisThread::sleep_for(100ms);
    
    // Start distance updates with longer interval
    distanceUpdate.attach(&updateDistance, 2s);
    
    update_display(true);
    
    while(1) {
        char key = getkey();
        
        if (key != 0) {
            printf("[INPUT] Key pressed: %c\n", key);
            
            switch(key) {
                case '2': 
                    scroll_up();
                    break;
                case '8': 
                    scroll_down();
                    break;
                case '5': 
                    select_option();
                    if (displayStartIndex + cursorPosition == 4) {
                        waterFlag = true;
                        printf("[STATE] Water enabled\n");
                    } else {
                        waterFlag = false;
                        printf("[STATE] Water disabled\n");
                    }
                    break;
            }
        }
        
        leak_detect();
        ThisThread::sleep_for(200ms);  // Reduced main loop delay
    }
}
