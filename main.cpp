#include "mbed.h"
#include <cstdio>
#include <cstring>

#include "sensors.hpp"
#include "wifi.hpp"
#include "lcd.h"
#include "lcdscroll.hpp"
#include "delay.hpp"
#include "pump.hpp"
Thread wifi(osPriorityNormal, 512);
DHT11 dht(PA_7);

int temp = 0;
int humidity = 0;
int brightness = 0;
float dist = 0.0f;
float moisture = 0.0f;
float tankFullPercent = 0.0f;

unsigned char key = 255;
BusIn Keypad_Data(PB_8, PB_9, PB_10, PB_11); // Keypad data pins
InterruptIn keypad_interrupt(PB_13);         // DA pin of the keypad for interrupt

EventQueue *queue = mbed_event_queue(); // event queue

const unsigned char lookupTable[] = {'1', '2', '3', 'F', '4', '5', '6', 'E', '7', '8', '9', 'D', 'A', '0', 'B', 'C'};

void keypad_ISR();
void myCallback();

void broadCastPage();

void broadCastPage() {
    setupWifi();
}

bool pumpRunning = false;

const unsigned char ledBarTable[] = {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF};
PortOut LEDBar(PortB, 0xFF);
int ledLevel;
float tempPercent;
float oldPercent = tankFullPercent;
#define LEAK_THRESH 7
#define MINDIST 5
#define MAXDIST 10

void updateSensors(){
     dht.readTemperatureHumidity(temp, humidity);
        dist = getDist();
        moisture = getMoist();
        brightness = getBright();

        tempPercent = (1 - (dist < 0 ? MAXDIST : dist - MINDIST)*1.0/(MAXDIST - MINDIST)) * 100;
        printf ("Obstacle is %.2f cm away!\n", dist);
        printf ("Brightness: %d\n", brightness);

        if (tempPercent > 100) tankFullPercent = 100;
        else if (tempPercent < 0) tankFullPercent = 0;
        else tankFullPercent = tempPercent;

        ledLevel = (tankFullPercent / 12.5);
        
        LEDBar = ledBarTable[ledLevel];

        if (oldPercent - tankFullPercent > LEAK_THRESH && !pumpRunning) {
            printf("LEAK, %.2f", oldPercent - tankFullPercent);
            startBuzzer();
        } else {
            pumpRunning = false;
        }

        oldPercent = tankFullPercent;
        if (key != 255) {
            if (key == 'D' && displayStartIndex + 1 < TOTAL_LINES) {  // Scroll Down
                scroll_down();
            } else if (key == 'E') {  // Scroll Up
                scroll_up();
            }
            select_option();
            key = 255;
        }
        if (moisture > 0.7){
            countdownTask();
        }
}
int main()
{
	lcd_init();
    update_display(true);
    initPump();
    
    wifi.start(broadCastPage);

    Keypad_Data.mode(PullNone);
    keypad_interrupt.rise(&keypad_ISR);

    while (true) {
        updateSensors();
        loadPage(temp, humidity, brightness, tankFullPercent, moisture);
    }
}

void keypad_ISR() {
    queue->call(myCallback);        // process in a different context
}

void myCallback() {
    if (relay.read() == 0) {
        printf("====== PUMP ON ======\n");
        key = 255;
    }

    while (key == 255) {
        key = lookupTable[Keypad_Data & Keypad_Data.mask()];
        printf("====== Called %d ======\n", key);
    }
}