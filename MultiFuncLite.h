/*
   MFSLite library created by Daniel Bickerstaff @ Bedford College.
   Reduced code footprint with core display functionality.
   
   Credits to the original library: MultiFuncShield from Kashif Baig @ Cohesive Computing
   
   Revisions:
   25/3/19: Added ScrollText routine - Scrolls text across display.
   30/3/19: Added function SetScrollTime - Sets Scrolling text sweep time.
   30/3/19: Added function SetTicker - Sets Ticker callback time interval.
   02/04/19: Added function attachCallback - Assign function in main sketch for callback.
   02/04/19: Added function RaiseCallback - Raise callback to main sketch.
   
   

*/
#include <TimerOne.h>

#ifndef MultiFuncLite_h_
#define MultiFuncLite_h_

#define MULTI_FUNCTION_LITE_LIB_1_2

#include "Arduino.h"

#define ENABLE  1
#define DISABLE  0

#define LED_1_PIN     13
#define LED_2_PIN     12
#define LED_3_PIN     11
#define LED_4_PIN     10
#define LATCH_PIN     4
#define CLK_PIN       7
#define DATA_PIN      8

#define DIGIT_1  1
#define DIGIT_2  2
#define DIGIT_3  4
#define DIGIT_4  8
#define DIGIT_ALL  15

#define LED_1  1
#define LED_2  2
#define LED_3  4
#define LED_4  8
#define LED_ALL  15


class MultiFuncLite
{
  
  public:
  
  // Point to function in main sketch to execute upon callback!
   void attachCallback(void (*isr)()) __attribute__((always_inline)) {
	 RaiseCallback = isr;
    }
  
    static void (*RaiseCallback)();
    static void isrDefaultUnused();
	// end of callback inclusion!!
	
	
    // Pointer to user interrupt with frequency of 1khz.
    void (*userInterrupt)() = NULL;
    
    // Initializes this instance using a TimerOne instance. A 1khz interrupt is attached. 
    void initialize(TimerOne *timer1);
    
    // Initializes this instance, but interrupt based features are not available.
    void initialize();
    
	// added return timer 1 tick value
    unsigned long dbReturnTick(); 

    // For internal use only.
    void isrCallBack();
	
	void SetTicker(int VarTick); // sets ticker callback time
	
	void SetScrollTime(int VarScrollTime); // sets scroll text sweep time
	
	void isrTick(); // Used in main sketch for 1 second tick!

	void EnableCallback(bool VarTickEnable); // simply sets flag to enable callback to main program.
	
    // Initiates a millisecond countdown timer.
    void setTimer (unsigned long thousandths);

    // Gets the current value of the countdown timer.
    unsigned long getTimer();

    // Initiates and waits for millisecond countdown timer to reach 0.
    void wait(unsigned long thousandths);
    
    // Writes to the LED digit display.
    void write(const char *textstring, byte rightJustify =0);
    void write(int integer);
    void write(float number, byte decimalPlaces = 1);
    
    // Manually refreshes the Led digit display.
    // Not to be used whilst interrupt based features are available.
    void manualDisplayRefresh();
    
    // Blinks the digits on the LED digit display.
    void blinkDisplay(byte digits,           // use bitwise or, e.g. DIGIT_1 | DIGIT_2
                      byte enabled = ENABLE      // turns on/off the blinking
                    );

    void setDisplayBrightness(byte level);  // 0 = max, 3 = min
    
    // Turns LEDs on or off.
    void writeLeds(byte leds,                // use bitwise or, e.g. LED_1 | LED_2
                   byte lit                  // ON or OFF
                   );

    // Blinks the LEDs.
    void blinkLeds(byte leds,                // use bitwise or, e.g. LED_1 | LED_2
                   byte enabled = ENABLE         // ON or OFF
                   );

	void ScrollText(String VarMessage);      // New scroll text routine
	

 protected:
	
	
	
	
    
  private:
    TimerOne *timer1;
    volatile byte timerReadInProgress = 0;
    volatile byte timerWriteInProgress = 0;
     
    volatile unsigned long timer_volatile = 0;    // count down timer 1000th of a second resolution.
    volatile unsigned long timer_safe = 0;
    
  
    byte displayIdx = 0;
    byte blinkEnabled = 0;  // least significant bits mapped to display digits.
    byte blinkState = 0;
    byte blinkCounter = 0;
    
    //byte ledControlMask = 0;       // soft enable / disable LED. Disable LEDs here if using PWM from TImerOne library. 
    byte ledState =0;              // least significant bits mapped to LEDs
    byte ledBlinkEnabled =0;       // least significant bits mapped to LEDs
    byte ledOutput=0;              // current led outputs (taking into consideration blink)
	
	// CALLBACKS

  
};


#endif

