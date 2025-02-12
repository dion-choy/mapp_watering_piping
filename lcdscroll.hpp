#include "mbed.h"
#include "lcd.h"
#include "keypad.h"

extern char *lines[];
extern int displayStartIndex;
extern const int TOTAL_LINES;

// Function to update LCD with cursor
extern void update_display(bool);

// Function to scroll down (cursor moves properly, updates display only when needed)
extern void scroll_down();

// Function to scroll up (cursor moves properly, updates display only when needed)
extern void scroll_up() ;

// Function to select the currently highlighted option
extern void select_option();
