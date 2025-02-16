#include "mbed.h"
#include "lcd.h"
#include "keypad.h"

const char countMsg[] = "Days till watering:  ";
const char daysMsg[] = "     Days";
static int daysRemaining = 0;
static bool countdownActive = false;
static Thread countdownThread;
static bool inCountdownMenu = false;
static Mutex countdownMutex;
static Timer countdownTimer;
static bool threadRunning = false;
static uint64_t lastUpdateTime = 0;

// Forward declarations
void displayDays(void);
void displayFullScreen(void);

void displayDays() {
    countdownMutex.lock();
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
    countdownMutex.unlock();
}

void countdownTask() {
    threadRunning = true;
    printf("[COUNTDOWN] Thread started\n");
    
    while(threadRunning) {
        countdownMutex.lock();
        bool isActive = countdownActive;
        int currentDays = daysRemaining;
        bool inMenu = inCountdownMenu;
        uint64_t localLastUpdate = lastUpdateTime;
        countdownMutex.unlock();

        uint64_t currentTime = countdownTimer.elapsed_time().count() / 1000; // ms

        if(isActive && currentDays > 0) {
            if(currentTime - localLastUpdate >= 5000) { // 5-second interval
                countdownMutex.lock();
                if(countdownActive && daysRemaining > 0) {
                    daysRemaining--;
                    lastUpdateTime = currentTime;
                    printf("[COUNTDOWN] Day decremented: %d days\n", daysRemaining);
                    
                    if(inCountdownMenu) {
                        displayDays(); // Only update number position
                    }
                    
                    if(daysRemaining == 0) {
                        countdownActive = false;
                        if(inCountdownMenu) {
                            lcd_write_cmd(0x01);
                            lcd_write_cmd(0x80);
                            const char* waterMsg = "Watering plants...  ";
                            for(int i = 0; i < 20; i++) {
                                lcd_write_data(waterMsg[i]);
                            }
                            thread_sleep_for(3000);
                            displayFullScreen(); // Return to countdown screen
                        }
                    }
                }
                countdownMutex.unlock();
            }
        }
        thread_sleep_for(100);
    }
}

void initCountdownThread() {
    printf("[INIT] Starting countdown thread\n");
    countdownThread.start(countdownTask);
}

void displayFullScreen() {
    countdownMutex.lock();
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
    countdownMutex.unlock();
}

void startCountdown() {
    printf("[MENU] Entering countdown menu\n");
    countdownMutex.lock();
    inCountdownMenu = true;
    countdownMutex.unlock();
    
    displayFullScreen();

    while(true) {
        char key = getkey();
        printf("[MENU] Key pressed: %c\n", key);
        
        if(key == 'A') {
            printf("[MENU] Exit requested\n");
            countdownMutex.lock();
            inCountdownMenu = false;
            countdownMutex.unlock();
            
            lcd_write_cmd(0x01);
            thread_sleep_for(10);
            return;
        }
        
        countdownMutex.lock();
        if(key >= '0' && key <= '9') {
            if(!countdownActive) {
                daysRemaining = key - '0';
                countdownActive = true;
                countdownTimer.reset();
                countdownTimer.start();
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
                countdownTimer.reset();
                lastUpdateTime = 0;
            }
            displayDays();
        }
        else if(key == 'D' && daysRemaining > 0) {
            daysRemaining = (countdownActive) ? daysRemaining - 1 : 0;
            if(!countdownActive && daysRemaining > 0) {
                countdownActive = true;
                countdownTimer.reset();
                lastUpdateTime = 0;
            }
            displayDays();
        }
        countdownMutex.unlock();
    }
}