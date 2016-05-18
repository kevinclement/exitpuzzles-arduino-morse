#include "morseLib.h"

// tmp
bool lastKeyerState = false;
unsigned long lastDebounceTime = 0;  // the last time the input pin was toggled
unsigned long debounceDelay = 20;
bool morseSignalState = false;
unsigned long markTime = 0;    // timers for mark and space in morse signal
unsigned long spaceTime = 0;   // E=MC^2 ;p
int morseSignals;              // nr of morse signals to send in one morse character
boolean gotLastSig = true;     // Flag that the last received morse signal is decoded as dot or dash
const int morseTreetop = 31;   // character position of the binary morse tree top.
int morseTableJumper = (morseTreetop + 1) / 2;
int morseTablePointer = morseTreetop;
char morseTable[] = "5H4S?V3I?F?U??2E?L?R???A?P?W?J1 6B?D?X?N?C?K?Y?T7Z?G?Q?M8??O9?0";
bool morseSpace = false;    // Flag to prevent multiple received spaces

// TMP: can move and embed in code?
unsigned long dotTime = 75;   // morse dot time length in ms
unsigned long dashTime = 300;
unsigned long wordSpace = 20000;

MorseLib::MorseLib(uint8_t pinIn, uint8_t speakerPin, bool echo)
{
  _pinIn = pinIn; 
  _speakerPin = speakerPin;
  _echo = echo;
}

void MorseLib::setup()
{
  pinMode(_pinIn, INPUT);
  digitalWrite(_pinIn, HIGH); // internal pullup resistor on

  // setup speaker
  pinMode(_speakerPin, OUTPUT);

  markTime = millis();
  spaceTime = millis();
}

char MorseLib::getChar()
{
  bool morseKeyer = !digitalRead(_pinIn); // inverted for active-low input

  // If the switch changed, due to noise or pressing:
  if (morseKeyer != lastKeyerState)
  {
    lastDebounceTime = millis(); // reset timer
  }

  if ((millis() - lastDebounceTime) > debounceDelay)
  {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
    morseSignalState = morseKeyer;
    // differentiante mark and space times
    if (morseSignalState) markTime = lastDebounceTime; else spaceTime = lastDebounceTime;
  }

  // Morse output, or a feedback when keying.
  if (_echo) {
    digitalWrite(_speakerPin, morseSignalState);
  }
 
  //  // Decode morse code
  if (!morseSignalState)
  {
    if (!gotLastSig)
    {
      if (morseTableJumper > 0)
      {
        // if pause for more than half a dot, get what kind of signal pulse (dot/dash) received last
        if (millis() - spaceTime > dotTime / 2)
        {
          // if signal for more than 1/4 dotTime, take it as a valid morse pulse
          if (spaceTime - markTime > dotTime / 4)
          {
            // if signal for less than half a dash, take it as a dot, else if not, take it as a dash
            // (dashes can be really really long...)
            if (spaceTime - markTime < dashTime / 2) morseTablePointer -= morseTableJumper;
            else morseTablePointer += morseTableJumper;
            morseTableJumper /= 2;
            gotLastSig = true;
          }
        }
      } else { // error if too many pulses in one morse character
        Serial.println("<ERROR: unrecognized signal!>");
        gotLastSig = true;
        morseTableJumper = (morseTreetop + 1) / 2;
        morseTablePointer = morseTreetop;
      }
    }
    // Write out the character if pause is longer than 2/3 dash time (2 dots) and a character received
    //if ((millis()-spaceTime >= (dotTime*2)) && (morseTableJumper < 16))
    if ((millis() - spaceTime >= 1000) && (morseTableJumper < 16))
    {
      char morseChar = morseTable[morseTablePointer];
      Serial.print(morseChar);

      morseTableJumper = (morseTreetop + 1) / 2;
      morseTablePointer = morseTreetop;
    }

    // Write a space if pause is longer than 2/3rd wordspace
    if (millis() - spaceTime > (wordSpace * 2 / 3) && morseSpace == false)
    {
      Serial.print(" ");
      morseSpace = true ; // space written-flag
    }

  } else {
    // while there is a signal, reset some flags
    gotLastSig = false;
    morseSpace = false;
  }


  // save last state of the morse signal for debouncing
  lastKeyerState = morseKeyer;

  return ' ';
}
