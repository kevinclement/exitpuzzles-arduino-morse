#ifndef MORSE_LIB_H
#define MORSE_LIB_H

#include "Arduino.h"

class MorseLib
{
public: 
  MorseLib(uint8_t pinIn, uint8_t speakerPin, bool echo);
  void setup();
  void reset();
  char getChar();

private:  
  // user configurable
  uint8_t _pinIn;
  uint8_t _speakerPin;
  bool _echo;

  // tracking
  unsigned long _markTime = 0;
  unsigned long _waitTime = 0;
  bool _pressed = false;
};

#endif // MORSE_LIB_H