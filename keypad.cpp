// LCDKeypadPwd.cpp
// Program to test LCD.
// The LCD display with two lines, 20 characters each.
// PORT A1: PA_15 : PA_8
// There are three control lines (PA_14:PA_12) and four data lines (PA_11 : PA_8).
// PA_14 - RS=0 Data represents Command, RS=1 Data represents Character
// PA_13 - RW=0 Writing into the LCD module
// PA_12 - E =1 Data is latched into LCD module during low to hight transition  
#undef __ARM_FP

#include "mbed.h"
#include "lcd.h"	
#include "keypad.h"	

unsigned char key, outChar;
unsigned char input = '0';
unsigned int delay = '0';
char Message1 [ ] = "Enter keypad input: ";	
const char openMessage[] = "    MANUAL INPUT    ";
const char openMessage2[] = "    OPENING VALVE  ";
const char nullMessage[] = " INVALID INPUT ";
const char delayMessage[] = " SET DELAY: ";
const char delayMessage2[] = " DELAY SET: ";


void displayMessage(const char* message) {
    for (int i = 0; message[i] != '\0'; i++) {
        lcd_write_data(message[i]);  
    }
}
char userInput(char input) {
    switch (input) {
        case '1': {
            lcd_write_cmd(0x80); 
            displayMessage(openMessage);
            lcd_write_cmd(0xC0); 
            displayMessage(openMessage2);
            printf("Valve is now OPEN.\n");
            return '1';
        }
        case '2': {
        lcd_write_cmd(0x01); // Clear LCD
        lcd_write_cmd(0x80); 
        displayMessage(delayMessage);
        char delay = getkey(); 
        lcd_write_data(delay);
        printf("The current delay is: %c\n", delay);
        delay *= 100;
        lcd_write_cmd(0xC0);  
        displayMessage(delayMessage2);
        lcd_write_data(delay);
        return delay;
        }   
        default: {
            displayMessage(nullMessage);
            return false;
        }
    }
}
  // Defining a 20 char string
