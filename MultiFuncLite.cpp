
/*
   MFSLite library created by Daniel Bickerstaff @ Bedford College.
   Reduced code footprint with core display functionality.
   
   Credits to the original library: MultiFuncShield from Kashif Baig @ Cohesive Computing
   
   Revisions:
   25/3/19: Added ScrollText routine - Scrolls text across display.
   30/3/19: Added function SetScrollTime - Sets Scrolling text sweep time.
   30/3/19: Added function SetTicker - Sets Ticker callback time interval.

*/

#include "MultiFuncLite.h"

MultiFuncLite MFSLite;

void (*MultiFuncLite::RaiseCallback)() = MultiFuncLite::isrDefaultUnused;

void MultiFuncLite::isrDefaultUnused()
{
}

// Display specific variables

const byte LED[] = {LED_1_PIN, LED_2_PIN, LED_3_PIN, LED_4_PIN};

/* Segment byte maps for numbers 0 to 9 */
const byte SEGMENT_MAP_DIGIT[] = {0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0X80,0X90};
/* Segment byte maps for alpha a-z */
const byte SEGMENT_MAP_ALPHA[] = {136, 131, 167, 161, 134, 142, 144, 139 ,207, 241, 182, 199, 182, 171, 163, 140, 152, 175, 146, 135, 227, 182, 182, 182, 145, 182};

/* Byte maps to select digit 1 to 4 */
const byte SEGMENT_SELECT[] = {0xF1,0xF2,0xF4,0xF8};
const char DISPLAY_OVERFLOW_ERROR = 'E';

const byte BLINK_ON_COUNT = 65;
const byte BLINK_OFF_COUNT = 20;

volatile byte displayMemory[4] = {255,255,255,255};

#define	DISPLAY_TIMER_SCALER_RELOAD	4

byte displayTimerScaler = DISPLAY_TIMER_SCALER_RELOAD;
byte displayBrightness = 0;


// New Variables added for functions DB
unsigned long DBTickCount = 0; // added to count time for Ticker callback, will rename!
int TickerTime = 1000; // used for callback time
int ScrollTime = 250; // Scroll text time default 250mS
bool FL_Enable_Callback = false; // to be added function wise for callback enable!!

// Misc methods and functions.
void isrWrapper ();
void WriteValueToSegment(byte Segment, byte Value);
byte AsciiToSegmentValue (byte ascii);
void writeLed(byte ledIdx, byte value);


void initShield()
{
    /* Set each LED pin to outputs */
  pinMode(LED[0], OUTPUT);
  pinMode(LED[1], OUTPUT);
  pinMode(LED[2], OUTPUT);
  pinMode(LED[3], OUTPUT);
  
  /* Turn all the LED's off */
  digitalWrite(LED[0], HIGH);
  digitalWrite(LED[1], HIGH);
  digitalWrite(LED[2], HIGH);
  digitalWrite(LED[3], HIGH);

  /* Set Segment display DIO pins to outputs */
  pinMode(LATCH_PIN,OUTPUT);
  pinMode(CLK_PIN,OUTPUT);
  pinMode(DATA_PIN,OUTPUT); 
  
  WriteValueToSegment(0,255);
  

}

void MultiFuncLite::SetTicker(int VarTick)
{
	TickerTime = VarTick;
}


void MultiFuncLite::SetScrollTime(int VarScrollTime)
{
	ScrollTime = VarScrollTime;
}

void MultiFuncLite::EnableCallback(bool VarTickEnable) // simply sets flag to enable callback to main program.

{
	DBTickCount = 0; // reset counter
	FL_Enable_Callback = VarTickEnable; // set ticker flag
	
}

// ----------------------------------------------------------------------------------------------------
void MultiFuncLite::initialize(TimerOne *timer1Instance)
{
  initShield();
  
  timer1 = timer1Instance;
  timer1->attachInterrupt(isrWrapper, 1000); // effectively, 1000 times per second 1 mS
 
}


void MultiFuncLite::initialize()
{
  initShield();
}


// ----------------------------------------------------------------------------------------------------
void MultiFuncLite::writeLeds(byte leds, byte lit)
{
  if (lit)
  {
    ledState = ledState | leds;
    //ledControlMask = ledControlMask | leds;
  }
  else
  {
    ledState = ledState & (255 - leds);
    //ledControlMask = ledControlMask & (255 - leds);
  }
}


// ----------------------------------------------------------------------------------------------------
void MultiFuncLite::blinkLeds(byte leds, byte enabled)
{
  if (enabled)
  {
    ledBlinkEnabled = ledBlinkEnabled | leds;
  }
  else
  {
    ledBlinkEnabled = ledBlinkEnabled & (255 - leds);
  }
}

// ----------------------------------------------------------------------------------------------------
void MultiFuncLite::setDisplayBrightness(byte level)
{
  displayBrightness = level >= DISPLAY_TIMER_SCALER_RELOAD ? DISPLAY_TIMER_SCALER_RELOAD-1 : level;
}


// ----------------------------------------------------------------------------------------------------
void MultiFuncLite::write(int integer)
{
  char displayText[5] = {' ',' ',' ',' ',0};
  
  if (integer > 9999 || integer < -999)
  {
    displayText[3] = DISPLAY_OVERFLOW_ERROR;
    write(displayText);
  }
  else if (integer == 0)
  {
    displayText[3] = '0';
    write (displayText);
  }
  else
  {
    byte sign = 0;
    if (integer < 0)
    {
      sign = 1;
      integer = integer * -1;
    }
    
    byte idx = 3;
    for (; idx >=0 && integer !=0; integer /= 10, idx--)
    {
      displayText[idx]=(integer % 10) + '0';
    }
    
    if (sign)
    {
      displayText[idx] = '-';
    }
    
    write (displayText);
  }
}


// ----------------------------------------------------------------------------------------------------
void MultiFuncLite::write(float number, byte decimalPlaces)
{
  char outstr[7];
  dtostrf(number, 4, decimalPlaces, outstr);
 
  if (strlen(outstr) > 5)
  {
    outstr[0] = DISPLAY_OVERFLOW_ERROR;
    outstr[1] = 0;
  }
  write(outstr,1);
}


// ----------------------------------------------------------------------------------------------------
void MultiFuncLite::write(const char *text, byte rightJustify)
{
  byte displayBuf[] = {0,0,0,0}, *pBuf = displayBuf;
  
  byte idx =0;
  
  for (; *text != 0 && idx < sizeof(displayBuf); text++)
  {
    byte offset = 0;
    
    if (*text == '.')
    {
      if (idx > 0)
      {
        displayBuf[idx-1] = displayBuf[idx-1] & 127;
      }
      else
      {
        displayBuf[idx] = AsciiToSegmentValue(*text);
        idx++;
      }
    }
    else
    {
      displayBuf[idx] = AsciiToSegmentValue(*text);
      idx++;
    }
  }
  
  for (; idx < sizeof(displayBuf); idx++)
  {
    displayBuf[idx] = 255;
  }
  
  // Copy display buffer to display memory
  
  if (rightJustify)
  {
    // right justify
    int i_src = sizeof(displayBuf)-1;
    int i_dst = sizeof(displayMemory)-1;
    
    for (; i_src >= 0 && displayBuf[i_src] == 255; i_src--) ;
      
    for (; i_src >= 0 && i_dst >= 0; i_src--, i_dst--)
    {
      displayMemory[i_dst] = displayBuf[i_src];
    }
    
    for (; i_dst >= 0; i_dst--)
    {
      displayMemory[i_dst] = 255;
    }
  }
  // left justify
  else
  {
    for (int i =0; i < sizeof(displayBuf); i++)
    {
      displayMemory[i] = displayBuf[i];
    }
  }
}


// ----------------------------------------------------------------------------------------------------
void MultiFuncLite::blinkDisplay(byte digits, byte enabled)
{
  if (enabled)
  {
    blinkEnabled = blinkEnabled | digits;
  }
  else
  {
    blinkEnabled = blinkEnabled & (255 - digits);
  }
}


// ----------------------------------------------------------------------------------------------------
void MultiFuncLite::setTimer(unsigned long thousandths)
{
  timerWriteInProgress = 1;
  timer_volatile = thousandths;
  timerWriteInProgress = 0;
  
  timerReadInProgress = 1;
  timer_safe = thousandths;
  timerReadInProgress = 0;
}


// ----------------------------------------------------------------------------------------------------
unsigned long MultiFuncLite::getTimer()
{
  unsigned long timer;
  timerReadInProgress = 1;
  timer = timer_safe;
  timerReadInProgress = 0;

  return timer;
}


// ----------------------------------------------------------------------------------------------------
void MultiFuncLite::wait(unsigned long thousandths)
{
  setTimer(thousandths);
  while (getTimer()) __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");
}


//-----------------------------------------------------------------------------------------------------
// Counts to Ticker period then raise callback to main sketch Beta
void MultiFuncLite::isrTick()
{
	 DBTickCount ++; // used to flag tick count value
  if (DBTickCount > TickerTime)
     {
	 DBTickCount = 0; // reset counter
     if (FL_Enable_Callback == true) // only send callback if enabled
	    {
		RaiseCallback(); // fire callback to main program  
	    }		 	
	 }
}


// ----------------------------------------------------------------------------------------------------
void MultiFuncLite::isrCallBack()
{
  byte displayEnabled = 1;
  
  displayTimerScaler--;
  
  if (displayTimerScaler == 0)
  {
    displayTimerScaler = DISPLAY_TIMER_SCALER_RELOAD;
    
    // Global bink control
    if (blinkEnabled || ledBlinkEnabled)
    {
      blinkCounter++;
      if (blinkState)
      {
        displayEnabled = 1;
        if (blinkCounter > BLINK_ON_COUNT)
        {
          blinkState = 0;
          blinkCounter = 0;
          displayEnabled = 0;
        }
      }
      else
      {
        displayEnabled = 0;
        if (blinkCounter > BLINK_OFF_COUNT)
        {
          blinkState = 1;
          blinkCounter = 0;
          displayEnabled = 1;
        }
      }
    }

    
    // Digit display blink control
    if (blinkEnabled & (1 << displayIdx))
    {
      if (displayEnabled)
      {
        WriteValueToSegment(displayIdx, displayMemory[displayIdx]);
      }
      else
      {
        WriteValueToSegment(displayIdx, 255);
      }
    }
    else
    {
      WriteValueToSegment(displayIdx, displayMemory[displayIdx]);
    }
    
    displayIdx++;
    if (displayIdx > sizeof(displayMemory)-1)
    {
      displayIdx = 0;
    }
    
  
    // LED output and blink control.
    
    byte ledOutputNew = (ledState & (displayEnabled ? 255 : 0) & ledBlinkEnabled) | (ledState & ~ledBlinkEnabled);
    
    if (ledOutputNew != ledOutput)
    {
      for (byte ledIdx = 0; ledIdx < 4; ledIdx++)
      {
        if ((ledOutputNew ^ ledOutput) & (1 << ledIdx))    // only set LED if its state has changed
        {
          if (ledBlinkEnabled & (1 << ledIdx))
          {
            //digitalWrite(LED[ledIdx], !(displayEnabled && ledState & (1 << ledIdx)));
            writeLed(ledIdx, !(displayEnabled && ledState & (1 << ledIdx)));
          }
          else
          {
            //digitalWrite(LED[ledIdx], !(ledState & (1 << ledIdx)));
            writeLed(ledIdx, !(ledState & (1 << ledIdx)));
          }
        }
      }
      ledOutput = ledOutputNew;
    }
  }
  else
  {
    // Handle display brightness
    if (displayTimerScaler == displayBrightness)
    {
      WriteValueToSegment(displayIdx == 0 ? 3 : displayIdx-1, 255);
    }
  }

 
  // Bump the count down timer.
  if (timer_volatile && !timerWriteInProgress)
  {
    timer_volatile--;
  }
  
  if (!timerReadInProgress)
  {
    timer_safe = timer_volatile;
  }
  
  if (userInterrupt)
  {
    userInterrupt();
  }
  
  
}


// ----------------------------------------------------------------------------------------------------
void MultiFuncLite::manualDisplayRefresh()
{
  WriteValueToSegment(displayIdx, displayMemory[displayIdx]);
  
  displayIdx++;
  if (displayIdx > sizeof(displayMemory)-1)
  {
    displayIdx = 0;
  }
}


// ----------------------------------------------------------------------------------------------------
// Added scroll text routine with padding.
  void MultiFuncLite::ScrollText (String VarMessage)
  {
	int MSG_Length;
    int Current_Position = 0;
    String Buffer = "";
	String TmpMSG = "    "; // pad
	TmpMSG += VarMessage;
	TmpMSG += "    "; // pad
    MSG_Length = TmpMSG.length();
    for (int i=0; i <= MSG_Length; i++)
        {
          Current_Position = i;
          Buffer = (TmpMSG.substring(Current_Position, Current_Position+4));
          write(Buffer.c_str());
          delay(ScrollTime); // Scroll delay
        }
  }
  
// ----------------------------------------------------------------------------------------------------
// TimerOne tick wrapper
void isrWrapper ()
{ // timerone interrupt handler
  MFSLite.isrCallBack(); // refresh display stuff
  // here we could add event tick has fired back to main sketch DB mod...
  MFSLite.isrTick(); // increment counter and raise callback if ticker period reached!
  
}



// ----------------------------------------------------------------------------------------------------
byte AsciiToSegmentValue (byte ascii)
{
  byte segmentValue = 182;
  
  if (ascii >= '0' && ascii <= '9')
  {
    segmentValue = SEGMENT_MAP_DIGIT[ascii - '0'];
  }
  else if (ascii >= 'a' && ascii <='z')
  {
    segmentValue = SEGMENT_MAP_ALPHA[ascii - 'a'];
  }
  else if (ascii >= 'A' && ascii <='Z')
  {
    segmentValue = SEGMENT_MAP_ALPHA[ascii - 'A'];
  }
  else
  {
    switch (ascii)
    {
      case '-':
        segmentValue = 191;
        break;
      case '.':
        segmentValue = 127;
        break;
      case '_':
        segmentValue = 247;
        break;
      case ' ':
        segmentValue = 255;
        break;
    }
  }
  
  return segmentValue;
}

/* ---------------------------------------------------------------------- */

#if defined(__AVR_ATmega328P__)      // Uno

  /* Write a value to one of the 4 digits of the display */
  void WriteValueToSegment(byte Segment, byte Value)
  {
    bitClear(PORTD, 4);

    for (uint8_t i = 0; i < 8; i++)  {
      bitWrite(PORTB, 0, !!(Value & (1 << (7 - i))));
      bitSet(PORTD, 7);
      bitClear(PORTD, 7);
    } 

    for (uint8_t i = 0; i < 8; i++)  {
      bitWrite(PORTB, 0, !!(SEGMENT_SELECT[Segment] & (1 << (7 - i))));
      bitSet(PORTD, 7);
      bitClear(PORTD, 7);          
    } 

    bitSet(PORTD, 4);
  }
  
  
  void writeLed(byte ledIdx, byte value)
  {
    switch (ledIdx)
    {
    case 0:
      bitWrite(PORTB, 5, value);
      break;
    case 1:
      bitWrite(PORTB, 4, value);
      break;
    case 2:
      bitWrite(PORTB, 3, value);
      break;
    case 3:
      bitWrite(PORTB, 2, value);
      break;
      }
  }

#endif
