/*
 * Bedford College Engineering MultiFuncLite Counter Example.
 * Written by Daniel Bickerstaff all rights reserved! 23/03/19
 * 
 * Basic counter example!
 */

/* 
----------------------------------------------------------------------------------------------
                                 DEFINE LIBRARYS USED!! 
----------------------------------------------------------------------------------------------
*/

#include <MultiFuncLite.h> // USING DB LITE LIBRARY
MultiFuncLite MFS;

  
/* 
----------------------------------------------------------------------------------------------
                                 DEFINE VARIABLE AND FLAGS USED!! 
----------------------------------------------------------------------------------------------
*/

int counter=0; // General counter

/* 
----------------------------------------------------------------------------------------------
                                 MAIN VOIDE SETUP ROUTNINE HERE!! 
----------------------------------------------------------------------------------------------
*/


void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial.println();
  Serial.println(F("MFSLite Counter Example"));
  Timer1.initialize();
  MFS.initialize(&Timer1);    // initialize multi-function shield library
  MFS.SetScrollTime(250); // set scrolling text time
  MFS.ScrollText (F("Starting"));
     
}


/* 
----------------------------------------------------------------------------------------------
                                 MAIN VOID LOOP ROUTINE!! 
----------------------------------------------------------------------------------------------
*/

void loop() 
{

  MFS.write(counter);
  delay(1000);
  counter ++;     

} // end void loop

