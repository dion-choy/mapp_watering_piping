#include "mbed.h"
#include <cstdio>
#include <cstring>

#include "sensors.hpp"
#include "wifi.hpp"
#include "lcd.h"
#include "keypad.h"
#include "lcdscroll.hpp"
#include "delay.hpp"
#include "leakdetect.hpp"  // Add header for leak detection

Thread wifi(osPriorityNormal, OS_STACK_SIZE/2);
Thread sensors(osPriorityNormal, OS_STACK_SIZE/2);
Thread countdownThread(osPriorityHigh);  // Give countdown higher priority
DHT11 dht(PA_7);

int temp = 0;
int humidity = 0;
int brightness = 0;
float dist = 0.0f;
float moisture = 0.0f;
float tankFullPercent = 0.0f;
bool pumpActive = false;  // Track pump status

DigitalOut decoderA0(PB_4);  // Least significant bit
DigitalOut decoderA1(PB_5);  // Middle bit
DigitalOut decoderA2(PB_6);  // Most significant bit
DigitalOut relay(PA_0);

void broadCastPage();
void updateCode();

void broadCastPage() {
    setupWifi();
    while (true) {
        loadPage(temp, humidity, brightness, tankFullPercent, moisture);
    }
}

void updateCode() {
    while (true) {
        dht.readTemperatureHumidity(temp, humidity);
        dist = getDist();
        moisture = getMoist();
        brightness = getBright();
        thread_sleep_for(1000);
        printf ("Obstacle is %.2f cm away!\n", dist);
        printf ("Brightness: %d\n", brightness);
    }
}

int main()
{
	lcd_init();
    
    // Start countdown thread first with high priority
    initCountdownThread();
    
    // Then start other threads
    sensors.start(updateCode);
    wifi.start(broadCastPage);

    float maxdist = 30.0;
    float mindist = 5.0;

    lcd_init(); // Initialize LCD
    update_display(true); // Show initial text with full refresh

    uint32_t lastKeyTime = 0;
    const uint32_t KEY_TIMEOUT = 5000;  // 5 seconds timeout for menu

    while (true) {
        float tempPercent = (1 - (dist - mindist)/(maxdist - mindist)) * 100;
        
        if (tempPercent > 100) tankFullPercent = 100;
        else if (tempPercent < 0) tankFullPercent = 0;
        else tankFullPercent = tempPercent;

        printf("The tank is %.2f %% full (LED level: %.2f)\n", tankFullPercent, tempPercent);
        printf("%s", ipBuf);

        // Check for key input (non-blocking)
        if(keypad_getkey_nb(&key)) {  // Assuming you have a non-blocking keypad function
            lastKeyTime = us_ticker_read() / 1000;  // Update last key press time
            
            if (key == 'D' && displayStartIndex + 1 < TOTAL_LINES) {
                scroll_down();
            } else if (key == 'E') {
                scroll_up();
            } else if (key == 'A') {
                select_option();
            }
        }
        
        // If no key pressed for timeout period, check for leaks
        uint32_t currentTime = us_ticker_read() / 1000;
        if(currentTime - lastKeyTime > KEY_TIMEOUT) {
            // Check for leaks
            if(check_for_leak(pumpActive)) {
                // If leak detected, show warning for a few seconds then return to menu
                thread_sleep_for(3000);
                update_display(true);  // Refresh menu display
            }
        }

        thread_sleep_for(100);  // Small delay to prevent busy waiting
    }
}