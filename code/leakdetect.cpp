#include "mbed.h"
#include "lcd.h"    
#include "sensors.hpp"

// Message strings for leak detection (only shown when leak detected)
const char Message1[] = "Warning! Water is";	
const char Message2[] = "leaking!";  

// Leak detection variables
static float referenceLevel = 0;
static float lastLevel = 0;
static uint32_t lastCheckTime = 0;
static bool pumpWasActive = false;
static bool leakDetected = false;

// Constants
const float LEAK_THRESHOLD = 2.0;     // cm decrease to trigger leak warning
const int LEAK_CHECK_INTERVAL = 5000;  // Check every 5 seconds
const int LEVEL_STABILIZE_TIME = 10000; // Time to wait after pump stops

bool check_for_leak(bool pumpActive) {
    uint32_t currentTime = us_ticker_read() / 1000;
    float currentLevel = getDist();
    
    // Initialize reference level on first call
    if (referenceLevel == 0) {
        referenceLevel = currentLevel;
        lastLevel = currentLevel;
        lastCheckTime = currentTime;
        printf("[LEAK] System initialized. Reference level: %.2f cm\n", referenceLevel);
        return false;
    }

    // Handle pump activity
    if (pumpActive) {
        pumpWasActive = true;
        referenceLevel = currentLevel;  // Update reference while pump runs
        return false;
    }

    // Wait for water level to stabilize after pumping
    if (pumpWasActive) {
        if (currentTime - lastCheckTime < LEVEL_STABILIZE_TIME) {
            return leakDetected;
        }
        referenceLevel = currentLevel;
        pumpWasActive = false;
        lastCheckTime = currentTime;
        return false;
    }

    // Check for leaks at regular intervals
    if (currentTime - lastCheckTime >= LEAK_CHECK_INTERVAL) {
        float totalChange = currentLevel - referenceLevel;

        // Only act if significant change detected and pump isn't running
        if (totalChange > LEAK_THRESHOLD && !pumpActive) {
            if (!leakDetected) {  // Only show warning on initial detection
                printf("[LEAK] Warning! Water loss detected: %.2f cm\n", totalChange);
                
                // Show warning on LCD only when leak first detected
                lcd_write_cmd(0x01);  // Clear any existing display
                lcd_write_cmd(0x80);
                for(int i = 0; Message1[i] != '\0'; i++) {
                    lcd_write_data(Message1[i]);
                }
                lcd_write_cmd(0xC0);
                for(int i = 0; Message2[i] != '\0'; i++) {
                    lcd_write_data(Message2[i]);
                }
            }
            leakDetected = true;
        } else {
            leakDetected = false;
        }

        lastLevel = currentLevel;
        lastCheckTime = currentTime;
    }

    return leakDetected;
}
