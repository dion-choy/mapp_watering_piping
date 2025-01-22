#include "mbed.h"
#include <cstdio>
#include <cstring>

#include "sensors.hpp"
#include "wifi.hpp"
#include "lcd.h"

Thread wifi;
DHT11 dht(PB_7);

int temp = 0;
int humidity = 0;
int brightness = 0;
float dist = 0.0f;
float moisture = 0.0f;
void broadCastPage(void) {
    setupWifi();
    while (true) {
        loadPage(temp, humidity, brightness, dist, moisture);
    }
}

int main()
{
	lcd_init();
    wifi.start(broadCastPage);
    while (true) {
        temp = dht.readTemperature();
        humidity = dht.readHumidity();
        dist = getDist();
        moisture = getMoist();
        brightness = getBright();
        // printf ("Obstacle is %.2f cm away!\n", dist);
        // printf ("Brightness: %d\n", brightness);
    }
}