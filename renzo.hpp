#ifndef RENZO_HPP
#define RENZO_HPP

// Declare external variables
extern bool waterFlag;
extern float objDist;
extern bool isInitialized;

// Declare constants if they need to be accessed by other files
extern const float LEAK_THRESHOLD;
extern const int MEASUREMENT_DELAY;
extern const int CONFIRMATION_READINGS;

// Declare external message strings if needed by other files
extern char Message[];
extern char Message1[];
extern char Message2[];

// Function declarations
bool leak_detect();

#endif 