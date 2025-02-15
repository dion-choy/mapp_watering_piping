#include "mbed.h"
#include "lcd.h"
#include "wifi.hpp"
#include "delay.hpp"

// Define multiple lines of text (5 rows total)
const char *lines[] = {
    "Disp Temp & Humid   ",
    "Disp Soil Moisture  ",
    "Disp IP address     ",
    "Watering Frequency  ",
    "Water NOW!!!!!      "
};

// Constants
int TOTAL_LINES = (sizeof(lines) / sizeof(lines[0]));  // Total stored lines
#define LCD_LINES 2  // LCD has 2 visible rows

// Track which row is currently highlighted (0 = first line, 1 = second line)
int cursorPosition = 0;
// Track which lines are being displayed
int displayStartIndex = 0;

bool selectedOption = false;

// Function prototypes
void select_option();
void update_display(bool);
void clear_lcd();

// Function to clear LCD only when changing to a new set of lines
void clear_lcd() {
    lcd_write_cmd(0x01);  // Clear display command
    thread_sleep_for(10); // Small delay to avoid flickering
}

// Function to update LCD with cursor
void update_display(bool full_refresh) {
    if (full_refresh) {
        clear_lcd(); // Only clear LCD when moving to a new set of lines
    }

    lcd_write_cmd(0x80);  // Move cursor to line 1
    lcd_write_data((cursorPosition == 0) ? '>' : ' '); // Cursor on first line
    for (int i = 0; i < 20; i++) {
        lcd_write_data(lines[displayStartIndex][i]); // Print first line
        if (!full_refresh) {
            break;
        }
    }

    lcd_write_cmd(0xC0);  // Move cursor to line 2
    if (displayStartIndex + 1 < TOTAL_LINES) {
        lcd_write_data((cursorPosition == 1) ? '>' : ' '); // Cursor on second line
        for (int i = 0; i < 20; i++) {
            lcd_write_data(lines[displayStartIndex + 1][i]); // Print second line
            if (!full_refresh) {
                break;
            }
        }
    } else {
        // If at the last line, show a blank space at the bottom
        lcd_write_data(' '); // No cursor on blank line
        for (int i = 0; i < 20; i++) {
            lcd_write_data(' '); // Fill empty space
        }
    }
    
    lcd_write_cmd(0x96);  // Clear display command
}

// Function to scroll down (cursor moves properly, updates display only when needed)
void scroll_down() {
    if (selectedOption) {
        return;
    }
    
    if (cursorPosition == 0) {
        // Move cursor to second line without clearing the screen
        cursorPosition = 1;
        update_display(false);
    } else {
        // Move to the next set of two lines only if possible
        if (displayStartIndex + 2 < TOTAL_LINES) {
            displayStartIndex += 2;
            cursorPosition = 0; // Reset cursor to first line of the new set
            update_display(true); // Clear screen only when changing displayed lines
        }
    }
}

// Function to scroll up (cursor moves properly, updates display only when needed)
void scroll_up() {
    if (selectedOption) {
        return;
    }
    
    if (cursorPosition == 1) {
        // Move cursor up to first line without clearing the screen
        cursorPosition = 0;
        update_display(false);
    } else {
        // Move to previous set of two lines only if possible
        if (displayStartIndex - 2 >= 0) {
            displayStartIndex -= 2;
            cursorPosition = 1; // Reset cursor to second line of the new set
            update_display(true); // Clear screen only when changing displayed lines
        }
    }
}

// Function to select the currently highlighted option
void select_option() {
    int selectedIndex = displayStartIndex + cursorPosition;
    printf("You selected: %s\n", lines[selectedIndex]); // Placeholder action

    if (selectedOption) {
        update_display(true);
        selectedOption = false;
        return;
    }

    clear_lcd();
    lcd_write_cmd(0x80);  // Move cursor to line 1
    // Placeholder: Add actual function calls for each option
    int i = 0;
    switch (selectedIndex) {
        case 0:
            printf("Displaying Temperature & Humidity...\n");
            break;
        case 1:
            printf("Displaying Soil Moisture Data...\n");
            break;
        case 2:
            printf("Displaying IP Address...\n");
            while (ipBuf[i] != '\0') {
                lcd_write_data(ipBuf[i]);
                i++;
            }
            break;
        case 3:
            printf("Setting Watering Frequency...\n");
            startCountdown();  // Call the countdown function
            break;
        case 4:
            printf("Activating Watering System NOW!!!\n");
            break;
        default:
            printf("Invalid selection\n");
            break;
    }
    selectedOption = true;
}