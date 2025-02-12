#undef __ARM_FP
#include <chrono>
using namespace std::chrono;
#include "mbed.h"

#define WAIT_TIME_US_0 10
#define WAIT_TIME_US_1 20
#define WAIT_TIME_US_2 58
#define WAIT_TIME_MS_0 25
#define WAIT_TIME_MS_1 30
#define WAIT_TIME_MS_2 100
#define WAIT_TIME_MS_3 300
#define WAIT_TIME_MS_4 500
#define WAIT_TIME_MS_5 1000
#define WAIT_TIME_MS_6 2000
#define WAIT_TIME_MS_7 3000
#define WAIT_TIME_MS_8 4000
#define ECHO_TIME_OUT 30
#define ECHO_MAX_TIME 25
// Comment out LED bar definitions
// #define DISPLAY_BAR_MASK 0x000000FF //PORT C0: PC_7 - PC_0
// #define DISPLAY_BAR_RESET 0x00000000 

// Input of IR sensor data
DigitalOut Trig(PC_5);
InterruptIn Echo(PC_6);
Timer echoTimer;
Timeout echoTimeOutTimer;
Ticker BlinkLED_PB14;

// Turn on/off the on-board LED: PB14
DigitalOut LED_PB14(PB_14);

// Add decoder pin definitions
DigitalOut decoderA0(PB_4);  // Least significant bit
DigitalOut decoderA1(PB_5);  // Middle bit
DigitalOut decoderA2(PB_6);  // Most significant bit

float objDistance = 0;
bool bDetectingObj = false;
bool bObjDetected = false;
bool bTimeOut = false;
bool bShortDistance = false;
bool bMediumDistance = false;
bool bLongDistance = false;
unsigned int echoTime_Obj = 0;
float dist, tankFullPercent;
float maxdist = 30.0;
float mindist = 5.0;


void ISR_EchoTimeOut()
{
    bTimeOut = true; // Set echo time out flag to true
}

void ISR_PC6_Echo_High(void)
{
    if (bDetectingObj)
    {
        echoTimer.reset();
        echoTimeOutTimer.attach(&ISR_EchoTimeOut, 30ms);
    }
}

void ISR_PC6_Echo_Low(void)
{
    if (bDetectingObj)
    {
        bObjDetected = true;
        bDetectingObj = false;
        echoTime_Obj = duration_cast<microseconds>(echoTimer.elapsed_time()).count();
        echoTimer.reset();
    }
}

void ToggleLED(void)
{
    LED_PB14 = !LED_PB14;
}

float getDist()
{
    Echo.rise(&ISR_PC6_Echo_High);
    Echo.fall(&ISR_PC6_Echo_Low);
    echoTimer.start();

    bObjDetected = false;
    bDetectingObj = false;
    bTimeOut = false;

    Trig = 0; // Drag the trigger signal to low first
    thread_sleep_for(WAIT_TIME_MS_1); // Maintain at low for a while
    Trig = 1;
    wait_us(WAIT_TIME_US_1); // Minimum 10us, here we apply 20us
    Trig = 0;

    bDetectingObj = true;

    while (!bObjDetected && bDetectingObj && !bTimeOut)
    {
        // Wait for interrupt
        __WFI();
    }

    echoTimeOutTimer.detach();

    if (bTimeOut) // If time out (i.e., 30ms, no obstacle is detected)
    {
        printf("Echo Time Out!\n");
        bShortDistance = false;
        bMediumDistance = false;
        bLongDistance = false;
        return maxdist; // Return maximum distance if timeout
    }

    if ((echoTime_Obj / 1000) > 25) // If echo time is greater than 25ms, it is out of range
    {
        bShortDistance = false;
        bMediumDistance = false;
        bLongDistance = false;
        return maxdist; // Return maximum distance if out of range
    }

    objDistance = (0.03432 * echoTime_Obj) / 2;

    // Update LED blinking based on distance
    if (objDistance < 30) // If obstacle is quite near, like 30cm
    {
        if (!bShortDistance)
        {
            bShortDistance = true;
            bMediumDistance = false;
            bLongDistance = false;
            BlinkLED_PB14.attach(&ToggleLED, 100ms);
        }
    }
    else if (objDistance < 60) // If obstacle is near, like 60cm
    {
        if (!bMediumDistance)
        {
            bShortDistance = false;
            bMediumDistance = true;
            bLongDistance = false;
            BlinkLED_PB14.attach(&ToggleLED, 300ms);
        }
    }
    else // If obstacle is not near
    {
        if (!bLongDistance)
        {
            bShortDistance = false;
            bMediumDistance = false;
            bLongDistance = true;
            BlinkLED_PB14.attach(&ToggleLED, 1000ms);
        }
    }
    
    return objDistance;
}
int main(){
    
    // Remove previous LED initialization
    // displayBarPort = DISPLAY_BAR_RESET;
   
    while(true){
        dist = getDist();
        tankFullPercent = (1 - (dist - mindist)/(maxdist - mindist)) * 100;
        
        if (tankFullPercent > 100) tankFullPercent = 100;
        if (tankFullPercent < 0) tankFullPercent = 0;

        // Convert percentage to LED level (0-7)
        uint8_t ledLevel = (tankFullPercent / 12.5);
        if(ledLevel > 7) ledLevel = 7;  // Ensure we don't exceed 7 (3-bit limit)
        
        // Set decoder pins according to binary value
        decoderA0 = (ledLevel & 0x01);       // Bit 0
        decoderA1 = (ledLevel & 0x02) >> 1;  // Bit 1
        decoderA2 = (ledLevel & 0x04) >> 2;  // Bit 2

        printf("The tank is %.2f %% full (LED level: %d)\n", tankFullPercent, ledLevel);
        thread_sleep_for(WAIT_TIME_MS_2);
    }
}
