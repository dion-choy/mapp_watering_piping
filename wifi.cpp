#include "mbed.h"
#include <cstdio>
#include <cstring>
#include <stdio.h>

#include "lcd.h"

// Maximum number of element the application buffer can contain
#define MAXIMUM_BUFFER_SIZE 1024
#define UART3_TX PC_10
#define UART3_RX PC_11
// Create a DigitalOutput object to toggle an LED
//PB_14-> send data, PB_15-> receive data
static DigitalOut led_PB14(PB_14);
static DigitalOut led_PB15(PB_15);
// Create a BufferedSerial object with a default baud rate.
static BufferedSerial serial_port(UART3_TX, UART3_RX);
//static BufferedSerial serial_port(UART2_TX, UART2_RX);
// Application buffer to send the data
char bufRx[MAXIMUM_BUFFER_SIZE] = {0};
char bufTx[MAXIMUM_BUFFER_SIZE] = {0};
char ipBuf[15] = {0};
char bufCommand[MAXIMUM_BUFFER_SIZE] = {0};
char brightnessStr[7];
char ipString[] = "+CIFSR:STAIP,\"";
uint32_t num1 = 0, num2 = 0, num3 = 0, num4 = 0;
int maxConn;

void setupWifi() {
    serial_port.set_baud(115200);
    serial_port.set_format(
    /* bits */ 8,
    /* parity */ BufferedSerial::None,
    /* stop bit */ 1
    );

    serial_port.set_flow_control(mbed::BufferedSerial::Disabled);
    //cmd: "AT" to check the status of ESP01
    sprintf (bufTx, "AT\r\n");
    thread_sleep_for(1000);

    num1 = serial_port.write(bufTx, strlen(bufTx));
    if(num1) {
        // Toggle the LED.
        led_PB14 = !led_PB14;
    }
    
    thread_sleep_for(1000);

    {
        num2 = serial_port.read(bufRx, sizeof(bufRx));
        if(num2) {
            // Toggle the LED.
            led_PB15 = !led_PB15;
            bufRx[num2] = '\0';
            printf("%s\n", bufRx);
        }
        thread_sleep_for(500);
    }

    sprintf (bufTx, "AT+CWMODE=1\r\n");
    num1 = serial_port.write(bufTx, strlen(bufTx));
    if(num1) {
        // Toggle the LED.
        led_PB14 = !led_PB14;
    }

    thread_sleep_for(700);
    
    {
        num2 = serial_port.read(bufRx, sizeof(bufRx));
        if(num2) {
            // Toggle the LED.
            led_PB15 = !led_PB15;
            bufRx[num2] = '\0';
            printf("%s\n", bufRx);
        }

        thread_sleep_for(500);
    }
    
    sprintf (bufTx, "AT+CIFSR\r\n");
    num1 = serial_port.write(bufTx, strlen(bufTx));
    if(num1) {
        // Toggle the LED.
        led_PB14 = !led_PB14;
    }
    
    thread_sleep_for(500);
    
    {
        num2 = serial_port.read(bufRx, sizeof(bufRx));
        if(num2) {
            // Toggle the LED.
            led_PB15 = !led_PB15;
            bufRx[num2] = '\0';
            printf("%s\n", bufRx);
        }
        
        thread_sleep_for(500);
    }
    
    char ssid[] = "Someone";
    char password[] = "dionchoy";
    snprintf (bufTx, sizeof(bufTx), "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, password);
    num1 = serial_port.write(bufTx, strlen(bufTx));
    if(num1) {
        // Toggle the LED.
        led_PB14 = !led_PB14;
    }
    
    thread_sleep_for(8000);
    
    {
        num2 = serial_port.read(bufRx, sizeof(bufRx));
        if(num2) {
            // Toggle the LED.
            led_PB15 = !led_PB15;
            bufRx[num2] = '\0';
            printf("%s\n", bufRx);
    }

    thread_sleep_for(200);
    }
    
    //thread_sleep_for(5000);
    sprintf (bufTx, "AT+CIFSR\r\n");
    num1 = serial_port.write(bufTx, strlen(bufTx));
    if (num1) {
        // Toggle the LED.
        led_PB14 = !led_PB14;
    }

    thread_sleep_for(700);
    
    {
        num2 = serial_port.read(bufRx, sizeof(bufRx));
        if(num2) {
            // Toggle the LED.
            led_PB15 = !led_PB15;
            bufRx[num2] = '\0';
            printf("%s\n", bufRx);

            for (int i=0; i<num2; i++) {
                int j;
                for (j=0; j<strlen(ipString); j++) {
                    if (ipString[j] != bufRx[i+j]) {
                        break;
                    }
                }
                if (j == strlen(ipString)) {
                    lcd_write_cmd(0x80);

                    int k = 0;
                    while (bufRx[i+j] != '"') {
                        ipBuf[k] = bufRx[i+j];
                        j++;
                        k++;
                    }
                    ipBuf[k] = '\0';
                }
            }
            printf("%s", ipBuf);
        }
        thread_sleep_for(200);
    }

    sprintf (bufTx, "AT+CIPMUX=1\r\n");
    num1 = serial_port.write(bufTx, strlen(bufTx));
    if(num1) {
        // Toggle the LED.
        led_PB14 = !led_PB14;
    }

    thread_sleep_for(700);
    
    {
        num2 = serial_port.read(bufRx, sizeof(bufRx));
        if(num2) {
            // Toggle the LED.
            led_PB15 = !led_PB15;
            bufRx[num2] = '\0';
            printf("%s\n", bufRx);
        }
        thread_sleep_for(200);
    }

    sprintf (bufTx, "AT+CIPSERVER=1,80\r\n");
    num1 = serial_port.write(bufTx, strlen(bufTx));
    if(num1) {
        // Toggle the LED.
        led_PB14 = !led_PB14;
    }

    thread_sleep_for(1000);
    
    {
        num2 = serial_port.read(bufRx, sizeof(bufRx));
        if(num2) {
            // Toggle the LED.
            led_PB15 = !led_PB15;
            bufRx[num2] = '\0';
            printf("%s\n", bufRx);
        }
        thread_sleep_for(200);
    }

    sprintf(bufTx, "AT+CIPSERVERMAXCONN?\r\n");
    num1 = serial_port.write(bufTx, strlen(bufTx));

    thread_sleep_for(200);
    {
        num2 = serial_port.read(bufRx, sizeof(bufRx));
        if(num2) {
            // Toggle the LED.
            bufRx[num2] = '\0';
            printf("%s\n", bufRx);
        }
        thread_sleep_for(500);
    }

    maxConn = bufRx[num2-7] - '0';
}

void loadPage(int temp, int humidity, int brightness, float percentFull, float moisture) {
    if (brightness == 0) {
        sprintf(brightnessStr, "Dark  ");
    } else if (brightness == 1) {
        sprintf(brightnessStr, "Dim   ");
    } else if (brightness == 2) {
        sprintf(brightnessStr, "Bright");
    }

    for (int i=0; i<maxConn; i++) {
        sprintf (bufTx, "AT+CIPSEND=%d,380\r\n", i);

        num1 = serial_port.write(bufTx, strlen(bufTx));
        if(num1) {
            // Toggle the LED.
            led_PB14 = !led_PB14;
        }

        {
            num2 = serial_port.read(bufRx, sizeof(bufRx));
            if(num2) {
                // Toggle the LED.
                bufRx[num2] = '\0';
                // printf("%s\n", bufRx);
            }
            thread_sleep_for(100);
        }

        sprintf(bufTx, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:312\r\n\r\n"
        "<!DOCTYPE html>"
        "<html lang=\"en\">"
        "<head>"
        "<meta charset=\"UTF-8\">"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
        "<meta http-equiv=\"refresh\" content=\"3;URL='/'\">"
        "<title>Document</title>"
        "</head>"
        "<body>"
        "Water Tank: %.2f&percnt;<br>"
        "Temperature: %d<br>"
        "Humdity: %d<br>"
        "Brightness: %s<br>"
        "Moisture: %.2f"
        "                                               "  //padding
        "\r\n", percentFull, temp, humidity, brightnessStr, moisture);
        num1 = serial_port.write(bufTx, strlen(bufTx));
        if(num1) {
            // Toggle the LED.
            led_PB14 = !led_PB14;
            printf ("Send %d characters to client.\n", num1);
        }
    }
}