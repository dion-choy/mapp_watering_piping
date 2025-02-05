#include "mbed.h"
#include <cstdio>
#include <cstring>

#include "sensors.hpp"
#include "wifi.hpp"
#include "lcd.h"

Thread wifi(osPriorityNormal, OS_STACK_SIZE/2);
Thread sensors(osPriorityNormal, OS_STACK_SIZE/2);
DHT11 dht(PA_4);

int temp = 0;
int humidity = 0;
int brightness = 0;
float dist = 0.0f;
float moisture = 0.0f;

void broadCastPage();
void updateCode();

void broadCastPage() {
    setupWifi();
    while (true) {
        loadPage(temp, humidity, brightness, dist, moisture);
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
        // printf ("Brightness: %d\n", brightness);
    }
}

int main()
{
	lcd_init();
    sensors.start(updateCode);
    wifi.start(broadCastPage);
    while (true) {
    }
}