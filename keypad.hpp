#ifndef KEYPAD_HPP
#define KEYPAD_HPP

// Function declarations
extern void displayMessage(const char*);
extern char userInput(char input);
extern char getkey();  // if this is defined in keypad.cpp

// Global variables
extern unsigned char key, outChar;
extern unsigned char input;
extern unsigned int delay;

#endif