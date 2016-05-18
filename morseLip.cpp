#include "morseLib.h"
#include <Bounce2.h>

// global defines
#define DASH_TIME 120
#define WAIT_THRESHOLD 1000

// tmp
unsigned long markTime = 0;
unsigned long waitTime = 0;
bool pressed = false;

// Debounce stuff
Bounce debouncer = Bounce(); 

// decode stuff
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

  // debounce setup
  debouncer.attach(_pinIn);
  debouncer.interval(20);
}

char MorseLib::getChar()
{
  // empty char
  char morseChar = '\0';

  // debounce button
  debouncer.update();

  // Get the updated value
  bool btnPressed = debouncer.read() == LOW; 

  // Output to speaker if enabled
  if (_echo) {
    digitalWrite(_speakerPin, btnPressed);
  }

  // being pressed, just track it so we can decode it when not pressed
  if (btnPressed) {

    // first time changed state, mark time this happened
    if (!pressed) {
      markTime = millis();
    }
    
    pressed = true;    
  } 
  else {

    // if we haven't decoded last press, then do that
    if (pressed) {
      
      // determine if it was a dot or a dash
      if (millis() - markTime > DASH_TIME) {
        morseTablePointer += morseTableJumper;
      } 
      else {
        morseTablePointer -= morseTableJumper;
      }
      morseTableJumper /= 2;

      // track how long we've been waiting for a press
      waitTime = millis();
    } else {
      
      // otherwise we're waiting, if this is first time, write down its time
      if (waitTime > 0 && millis() - waitTime > WAIT_THRESHOLD) {
        morseChar = morseTable[morseTablePointer];
        morseTableJumper = (morseTreetop + 1) / 2;
        morseTablePointer = morseTreetop;

        waitTime = 0;
      }
    }    

    pressed = false;
  }

  return morseChar;
}
