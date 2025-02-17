#include "mbed.h"
#include "lcd.h"
#include "sensors.hpp"
#include "leakdetect.hpp"
#include "keypad.h"

bool pumpActive = false;  // Global pump status flag

int main() {
    printf("\nStarting Leak Detection Test\n");
    printf("Move hand closer to sensor to simulate water loss\n");
    printf("Press 'C' to simulate pump activity\n");
    printf("Press 'A' to exit test\n");
    
    lcd_init();
    
    // Initial display
    lcd_write_cmd(0x01);
    lcd_write_cmd(0x80);
    const char* testMsg = "Leak Detection:";
    for(int i = 0; testMsg[i] != '\0'; i++) {
        lcd_write_data(testMsg[i]);
    }
    
    // Store initial water level
    float currentLevel = getDist();
    printf("Initial water level: %.2f cm\n", currentLevel);
    
    while(true) {
        // Get current distance reading
        float newLevel = getDist();
        printf("Current level: %.2f cm\n", newLevel);
        
        // Check for keypad input (non-blocking)
        char key = getkey();
        
        if(key == 'A') {
            printf("[TEST] Exiting leak detection test\n");
            lcd_write_cmd(0x01);
            return 0;
        }
        else if(key == 'C') {  // Simulate pump activity
            pumpActive = true;
            printf("\n[TEST] Pump activated\n");
            thread_sleep_for(2000);
            pumpActive = false;
            currentLevel = getDist();  // Reset reference level after pump
            printf("[TEST] New water level after pump: %.2f cm\n", currentLevel);
        }
        
        // Check for leaks using actual sensor data
        bool leakStatus = check_for_leak(pumpActive);
        
        // Display current status on second line
        lcd_write_cmd(0xC0);
        if(leakStatus) {
            const char* errorMsg = "LEAK DETECTED!    ";
            for(int i = 0; errorMsg[i] != '\0'; i++) {
                lcd_write_data(errorMsg[i]);
            }
            printf("[ERROR] Water loss detected!\n");
            printf("[ERROR] Level change: %.2f cm\n", newLevel - currentLevel);
            printf("[ERROR] System may have a leak!\n");
        } else {
            char statusMsg[20];
            sprintf(statusMsg, "Level: %.1f cm    ", newLevel);
            for(int i = 0; statusMsg[i] != '\0' && i < 16; i++) {
                lcd_write_data(statusMsg[i]);
            }
        }
        
        thread_sleep_for(100);  // Small delay between readings
    }
} 