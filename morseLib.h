#ifndef MORSE_LIB_H
#define MORSE_LIB_H

#include "Arduino.h"

class MorseLib
{
public: 
  MorseLib(uint8_t pinIn, uint8_t speakerPin, bool echo);
  void setup();
  char getChar();

private:  
  uint8_t _pinIn;
  uint8_t _speakerPin;
  bool _echo;
};

#endif // MORSE_LIB_H
