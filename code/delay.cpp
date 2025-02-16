#include "mbed.h"
#include "lcd.h"
#include "key.hpp"
#include "lcdscroll.hpp"
#include "pump.hpp"

#include <chrono>

const char countMsg[] = "Days till watering:  ";
const char daysMsg[] = "     Days";
static int daysRemaining = 0;
static bool countdownActive = false;
static Thread countdownThread;
static Timer countdownTimer;
static bool threadRunning = false;
static uint64_t lastUpdateTime = 0;

// Forward declarations
void displayDays(void);
void displayFullScreen(void);

void displayDays() {
    printf("[DISPLAY] Updating display: %d days remaining\n", daysRemaining);
    
    // Update only the number position
    lcd_write_cmd(0xC5);
    thread_sleep_for(1);
    
    if(daysRemaining < 10) {
        lcd_write_data('0' + daysRemaining);
        lcd_write_data(' ');
    } else {
        lcd_write_data('0' + (daysRemaining / 10));
        lcd_write_data('0' + (daysRemaining % 10));
    }
}

void countdownTask() {
    printf("[COUNTDOWN] Thread running\n");
    bool isActive = countdownActive;
    int currentDays = daysRemaining;
    uint64_t localLastUpdate = lastUpdateTime;

    uint64_t currentTime = duration_cast<std::chrono::milliseconds> (countdownTimer.elapsed_time()).count() / 1000; // ms

    if(isActive && currentDays > 0) {
        printf("[COUNTDOWN] Day decremented: %d days\n", currentDays);
        printf("[COUNTDOWN] Day decremented: %llu days\n", currentTime);
        if(currentTime - localLastUpdate >= 5) { // 5-second interval
            if(countdownActive && daysRemaining > 0) {
                daysRemaining--;
                lastUpdateTime = currentTime;
                printf("[COUNTDOWN] Day decremented: %d days\n", daysRemaining);
                
                if(selectedOption) {
                    displayDays(); // Only update number position
                }
                
                if(daysRemaining == 0) {
                    countdownActive = false;
                    if(selectedOption) {
                        lcd_write_cmd(0x01);
                        lcd_write_cmd(0x80);
                        const char* waterMsg = "Watering plants...  ";
                        for(int i = 0; i < 20; i++) {
                            lcd_write_data(waterMsg[i]);
                        }
                        startPump();
                        thread_sleep_for(3000);
                        displayFullScreen(); // Return to countdown screen
                    }
                }
            }
        }
    }
}

void displayFullScreen() {
    lcd_write_cmd(0x01);
    thread_sleep_for(10);
    
    lcd_write_cmd(0x80);
    for(int i = 0; i < 20; i++) {
        lcd_write_data(countMsg[i]);
    }
    
    lcd_write_cmd(0xC6);
    for(int i = 0; i < 9; i++) {
        lcd_write_data(daysMsg[i]);
    }
    
    displayDays();
}

void startCountdown() {
    printf("[MENU] Entering countdown menu\n");
    if (key == 'A') {
        displayFullScreen();
    }

    if(key >= '0' && key <= '9') {
        if(!countdownActive) {
            daysRemaining = key - '0';
            countdownActive = true;
            lastUpdateTime = 0;
            printf("[MENU] Started countdown: %d days\n", daysRemaining);
        }
        else if(daysRemaining < 10) {
            daysRemaining = daysRemaining * 10 + (key - '0');
            if(daysRemaining > 99) daysRemaining = 99;
            printf("[MENU] Updated days: %d\n", daysRemaining);
        }
        displayDays();
    }
    else if(key == 'E' && daysRemaining < 99) {
        daysRemaining = (countdownActive) ? daysRemaining + 1 : 1;
        if(!countdownActive) {
            countdownActive = true;
            lastUpdateTime = 0;
        }
        displayDays();
    }
    else if(key == 'D' && daysRemaining > 0) {
        daysRemaining = (countdownActive) ? daysRemaining - 1 : 0;
        if(!countdownActive && daysRemaining > 0) {
            countdownActive = true;
            lastUpdateTime = 0;
        }
        displayDays();
    }
    countdownTimer.reset();
    countdownTimer.start();


    if (key != 255) {
        printf("[MENU] Key pressed: %c\n", key);
        key = 255;
    }
}