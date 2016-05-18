#include <LiquidCrystal_I2C.h>
#include <InputDebounce.h>
#include "MorseLib.h"

const int morseInPin = 2;      // The Morse keyer button
const int clearButtonPin = 3;
const int morseOutPin =  7;    // For Morse code output
unsigned long lcdTimeOn = 0;   // last time we keyed
unsigned long lcdTimeSleep = 300000;   // when to sleep lcd
int charCount = 0;
int charLimit = 15;

boolean lastClearButton = false;
boolean clearButtonState = false;
boolean lcdLightOn = false;
unsigned long lastDebounceButton = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2);  // set the LCD address to 0x20 for a 16 chars and 2 line display
static InputDebounce buttonTest; // not enabled yet, setup has to be called later
static unsigned int buttonTest_OnTimeLast = 0;

MorseLib ml(morseInPin, morseOutPin, true);

void buttonTest_releasedCallback()
{
  // handle released state
  Serial.println("LOW");
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Morse decoder by kevinc");

  // setup morse button
  ml.setup();  

  // setup clear button
  pinMode(clearButtonPin, INPUT_PULLUP);
  buttonTest.setup(clearButtonPin);
  buttonTest.registerCallbacks(NULL, buttonTest_releasedCallback, NULL);
 
  // setup the lcd
  lcd.init();
  lcd.cursor();  
}

// TODO:
//  morse to library to clean this up
//  morse lib to use debouce lib
//  pin to #define
//  add back lcd light logic
//  add back lcd limit logic
//  clean up more library so it doesn't need all the extra space and such
//  cleanup button press debounce
//  move morse over to debounce library
//  clear on button press
//  debug any issues
//  check for proper password 'polo'
//    if correct
//      buzzer happy beep
//      set high line on relay to open magnet
//  after 5 minutes clear screen
//    when wake up should be reset
//  clear is messing with cursor
//  add reset like password when puzzle is over

void loop()
{

  // char morseChar = morse.getChar();
  // if (morseChar != '')
  // {
  //   // handle morse character
  // }

  // TODO: ADD BACK
  // if (!lcdLightOn) {
  //      lcd.backlight();
  //      lcdLightOn = true;
  //  }

  // TODO: ADD BACK
  //charCount++;
  //    if (charCount > charLimit) {
  //      lcd.setCursor(charLimit, 0);
  //    }


  
  buttonTest.process(millis());

  if (millis() - lcdTimeOn > lcdTimeSleep) {
    lcdLightOn = false;
    lcd.clear();
    lcd.noBacklight();
  }

  char morseChar = ml.getChar();

  //boolean morseKeyer = !digitalRead(morseInPin); // inverted for active-low input

//  // If the switch changed, due to noise or pressing:
//  if (morseKeyer != lastKeyerState)
//  {
//    lastDebounceTime = millis(); // reset timer
//  }

  // debounce the morse keyer
//  if ((millis() - lastDebounceTime) > debounceDelay)
//  {
//    // whatever the reading is at, it's been there for longer
//    // than the debounce delay, so take it as the actual current state:
//    morseSignalState = morseKeyer;
//    // differentiante mark and space times
//    if (morseSignalState) markTime = lastDebounceTime; else spaceTime = lastDebounceTime;
//  }
  // Morse output, or a feedback when keying.
//  if (!sendingMorse && morseEcho) {
//    digitalWrite(morseOutPin, morseSignalState);
//  }
//
//  // Send Morse signals to output
//  if (sendingMorse)
//  {
//    switch (morseSignal[sendingMorseSignalNr])
//    {
//      case '.': // Send a dot (actually, stop sending a signal after a "dot time")
//        if (millis() - sendMorseTimer >= dotTime)
//        {
//          digitalWrite(morseOutPin, LOW);
//          sendMorseTimer = millis();
//          morseSignal[sendingMorseSignalNr] = 'x'; // Mark the signal as sent
//        }
//        break;
//      case '-': // Send a dash (same here, stop sending after a dash worth of time)
//        if (millis() - sendMorseTimer >= dashTime)
//        {
//          digitalWrite(morseOutPin, LOW);
//          sendMorseTimer = millis();
//          morseSignal[sendingMorseSignalNr] = 'x'; // Mark the signal as sent
//        }
//        break;
//      case 'x': // To make sure there is a pause between signals and letters
//        if (sendingMorseSignalNr < morseSignals - 1)
//        {
//          // Pause between signals in the same letter
//          if (millis() - sendMorseTimer >= dotTime)
//          {
//            sendingMorseSignalNr++;
//            digitalWrite(morseOutPin, HIGH); // Start sending the next signal
//            sendMorseTimer = millis();       // reset the timer
//          }
//        } else {
//          // Pause between letters
//          if (millis() - sendMorseTimer >= dashTime)
//          {
//            sendingMorseSignalNr++;
//            sendMorseTimer = millis();       // reset the timer
//          }
//        }
//        break;
//      case ' ': // Pause between words (minus pause between letters - already sent)
//      default:  // Just in case its something else
//        if (millis() - sendMorseTimer > wordSpace - dashTime) sendingMorse = false;
//    }
//    if (sendingMorseSignalNr >= morseSignals) sendingMorse = false; // Ready to encode more letters
//  }
//


//  // Decode morse code
//  if (!morseSignalState)
//  {
//    if (!gotLastSig)
//    {
//      if (!lcdLightOn) {
//        lcd.backlight();
//        lcdLightOn = true;
//      }
//
//      lcdTimeOn = millis();
//      if (morseTableJumper > 0)
//      {
//        // if pause for more than half a dot, get what kind of signal pulse (dot/dash) received last
//        if (millis() - spaceTime > dotTime / 2)
//        {
//          // if signal for more than 1/4 dotTime, take it as a valid morse pulse
//          if (spaceTime - markTime > dotTime / 4)
//          {
//            // if signal for less than half a dash, take it as a dot, else if not, take it as a dash
//            // (dashes can be really really long...)
//            if (spaceTime - markTime < dashTime / 2) morseTablePointer -= morseTableJumper;
//            else morseTablePointer += morseTableJumper;
//            morseTableJumper /= 2;
//            gotLastSig = true;
//          }
//        }
//      } else { // error if too many pulses in one morse character
//        Serial.println("<ERROR: unrecognized signal!>");
//        gotLastSig = true;
//        morseTableJumper = (morseTreetop + 1) / 2;
//        morseTablePointer = morseTreetop;
//      }
//    }
//    // Write out the character if pause is longer than 2/3 dash time (2 dots) and a character received
//    //if ((millis()-spaceTime >= (dotTime*2)) && (morseTableJumper < 16))
//    if ((millis() - spaceTime >= 1000) && (morseTableJumper < 16))
//    {
//      char morseChar = morseTable[morseTablePointer];
//      Serial.print(morseChar);
//      lcd.print(morseChar);
//
//      charCount++;
//      if (charCount > charLimit) {
//        lcd.setCursor(charLimit, 0);
//      }
//
//      morseTableJumper = (morseTreetop + 1) / 2;
//      morseTablePointer = morseTreetop;
//    }
//
//    // Write a space if pause is longer than 2/3rd wordspace
//    if (millis() - spaceTime > (wordSpace * 2 / 3) && morseSpace == false)
//    {
//      Serial.print(" ");
//      morseSpace = true ; // space written-flag
//    }
//
//  } else {
//    // while there is a signal, reset some flags
//    gotLastSig = false;
//    morseSpace = false;
//  }

//  // save last state of the morse signal for debouncing
//  lastKeyerState = morseKeyer;
}

