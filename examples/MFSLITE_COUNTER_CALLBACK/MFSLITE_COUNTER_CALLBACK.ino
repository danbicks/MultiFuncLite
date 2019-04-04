/*
 * Bedford College Engineering MultiFuncLite Counter Callback Example.
 * Written by Daniel Bickerstaff all rights reserved! 23/03/19
 * 
 * This example fires a callback event in the main sketch generated
 * from the MultiFuncLite library. This is very useful as now timer1
 * used by the core library can be used in your main program as well for
 * other timing applications.
 * Remember minimum timer1 tick is at 1mS!
 * 
 */

/* 
----------------------------------------------------------------------------------------------
                                 DEFINE LIBRARYS USED!! 
----------------------------------------------------------------------------------------------
*/

#include <MultiFuncLite.h> // NOW USING DB LITE LIBRARY

MultiFuncLite MFS;

  
/* 
----------------------------------------------------------------------------------------------
                                 DEFINE VARIABLE AND FLAGS USED!! 
----------------------------------------------------------------------------------------------
*/

int counter=0; // USED FOR COUNT VALUE!

/* 
----------------------------------------------------------------------------------------------
                                 MAIN VOID SETUP ROUTINE HERE!! 
----------------------------------------------------------------------------------------------
*/



void setup() {
 
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("MFSLite Counter Example"));
  Timer1.initialize();
  MFS.initialize(&Timer1);    // initialize library
  MFS.SetScrollTime(250); // set scrolling text time
  MFS.ScrollText (F("Starting"));  
  digitalWrite(13,LOW); // TURN ON MFSLITE LED

  //Set our callback function
  MFS.SetTicker(150); // sets ticker time to 150 mS interrupt
  MFS.attachCallback(onTicker); // Assign Callback handler routine.
  MFS.EnableCallback(true); // enable callback 

  
  delay(1000);
       
}


/* 
----------------------------------------------------------------------------------------------
                                 MAIN VOID LOOP ROUTINE!! 
----------------------------------------------------------------------------------------------
*/

void loop() 
{

  MFS.write(counter);
  delay(250);
  counter ++;     

  if (counter == 100)
     {
      MFS.EnableCallback(false); // Turn off callback
      MFS.ScrollText (F("Callbac off"));
     }


  if (counter == 200)
     {
      MFS.SetTicker(80); // sets ticker time to 80 mS faster interrupt!
      MFS.ScrollText (F("Callbac on")); 
      MFS.EnableCallback(true); // Turn on callback
     }
        
} // end void loop



// Callback function called, run code in onTicker routine!
void onTicker() {
  digitalWrite(13, !digitalRead(13)); // TOGGLE LED PIN 13
}

