#undef __ARM_FP
#include <chrono>
using namespace std::chrono;
#include "mbed.h"

DigitalOut Trig(PC_5);
InterruptIn Echo(PC_6);
Timer echoTimer;
Timeout echoTimeOutTimer;
Ticker BlinkLED_PB14;

float objDistance = 0;
bool bDetectingObj = false;
bool bObjDetected = false;
bool bTimeOut = false;
bool bShortDistance = false;
bool bMediumDistance = false;
bool bLongDistance = false;
unsigned int echoTime_Obj = 0;

void ISR_EchoTimeOut() {
    bTimeOut = true;  // set echo time out flag to be true
}
void ISR_PC6_Echo_High(void) {
    if (bDetectingObj) {
        echoTimer.reset();
        echoTimeOutTimer.attach(&ISR_EchoTimeOut, 30ms);
    }
}
void ISR_PC6_Echo_Low(void) {
    if (bDetectingObj) {
        bObjDetected = true;
        bDetectingObj = false;
        echoTime_Obj =
            duration_cast<microseconds>(echoTimer.elapsed_time()).count();
        echoTimer.reset();
    }
}

float getDist() {
    Echo.rise(&ISR_PC6_Echo_High);
    Echo.fall(&ISR_PC6_Echo_Low);
    echoTimer.start();
    while (true) {
        bObjDetected = false;
        bDetectingObj = false;
        bTimeOut = false;
        
        Trig = 0;  // drag the trigger signal to low first
        thread_sleep_for(30);  // maintain at low for a while

        Trig = 1;
        wait_us(20);  // minimum 10us, here we apply 20us
        Trig = 0;

        bDetectingObj = true;
        while (!bObjDetected && bDetectingObj && !bTimeOut) {
            printf("looping");
            __WFI();
        }

        echoTimeOutTimer.detach();
        if (bTimeOut)  // if time out (i.e., 30ms, no obstacle is detected)
        {
            printf("Echo Time Out !\n");
            bShortDistance = false;
            bMediumDistance = false;
            bLongDistance = false;
            return -1;
        }
        if ((echoTime_Obj / 1000) > 25) { // if echo time is greater than 25ms, it is out of the range.
            bShortDistance = false;
            bMediumDistance = false;
            bLongDistance = false;
            return -1;
        }

        return objDistance = (0.03432 * echoTime_Obj) / 2;
    }
}
