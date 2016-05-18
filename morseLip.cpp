#include "morseLib.h"

#define DOT_TIME 75
#define DASH_TIME 300

// tmp

// Debounce stuff
bool lastKeyerState = false;
unsigned long lastDebounceTime = 0;  // the last time the input pin was toggled
unsigned long debounceDelay = 20;
bool morseSignalState = false;

unsigned long markTime = 0;    // timers for mark and space in morse signal
unsigned long spaceTime = 0;   // E=MC^2 ;p
boolean gotLastSig = true;     // Flag that the last received morse signal is decoded as dot or dash

// translation stuff
char morseTable[] = "5H4S?V3I?F?U??2E?L?R???A?P?W?J1 6B?D?X?N?C?K?Y?T7Z?G?Q?M8??O9?0";
const int morseTreetop = 31;   // character position of the binary morse tree top.
int morseTableJumper = (morseTreetop + 1) / 2;
int morseTablePointer = morseTreetop;

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
  // empty char
  char morseChar = '\0';
  
  // ## DEBOUNCE ##################################################################################
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

  // ##############################################################################################

  // Morse output, or a feedback when keying.
  if (_echo) {
    digitalWrite(_speakerPin, morseSignalState);
  }
 
  // Decode morse code
  if (!morseSignalState)
  {
    if (!gotLastSig)
    {
      if (morseTableJumper > 0)
      {
        // if pause for more than half a dot, get what kind of signal pulse (dot/dash) received last
        if (millis() - spaceTime > DOT_TIME / 2)
        {
          // if signal for more than 1/4 dotTime, take it as a valid morse pulse
          if (spaceTime - markTime > DOT_TIME / 4)
          {
            // if signal for less than half a dash, take it as a dot, else if not, take it as a dash
            // (dashes can be really really long...)
            if (spaceTime - markTime < DASH_TIME / 2) morseTablePointer -= morseTableJumper;
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
    if ((millis() - spaceTime >= 1000) && (morseTableJumper < 16))
    {
      morseChar = morseTable[morseTablePointer];
      //Serial.print(morseChar);

      morseTableJumper = (morseTreetop + 1) / 2;
      morseTablePointer = morseTreetop;
    }
  } else {
    // while there is a signal, reset some flags
    gotLastSig = false;
  }

  // save last state of the morse signal for debouncing
  lastKeyerState = morseKeyer;

  return morseChar;
}
