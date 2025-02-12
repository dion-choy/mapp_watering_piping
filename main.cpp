#include "mbed.h"
#include <cstdio>
#include <cstring>

#include "sensors.hpp"
#include "wifi.hpp"
#include "lcd.h"

Thread wifi(osPriorityNormal, OS_STACK_SIZE/2);
Thread sensors(osPriorityNormal, OS_STACK_SIZE/2);
DHT11 dht(PA_7);

int temp = 0;
int humidity = 0;
int brightness = 0;
float dist = 0.0f;
float moisture = 0.0f;
float tankFullPercent = 0.0f;

DigitalOut decoderA0(PB_4);  // Least significant bit
DigitalOut decoderA1(PB_5);  // Middle bit
DigitalOut decoderA2(PB_6);  // Most significant bit

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
    sensors.start(updateCode);
    wifi.start(broadCastPage);

    float maxdist = 30.0;
    float mindist = 5.0;

    while (true) {
        float tempPercent = (1 - (dist - mindist)/(maxdist - mindist)) * 100;
        
        if (tempPercent > 100) tankFullPercent = 100;
        else if (tempPercent < 0) tankFullPercent = 0;
        else tankFullPercent = tempPercent;

        // // Convert percentage to LED level (0-7)
        // uint8_t ledLevel = (tankFullPercent / 12.5);
        // if(ledLevel > 7) ledLevel = 7;  // Ensure we don't exceed 7 (3-bit limit)
        
        // // Set decoder pins according to binary value
        // decoderA0 = (ledLevel & 0x01);       // Bit 0
        // decoderA1 = (ledLevel & 0x02) >> 1;  // Bit 1
        // decoderA2 = (ledLevel & 0x04) >> 2;  // Bit 2

        printf("The tank is %.2f %% full (LED level: %.2f)\n", tankFullPercent, tempPercent);
    }
}