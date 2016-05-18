#ifndef MORSE_LIB_H
#define MORSE_LIB_H

#include "Arduino.h"

class MorseLib
{
public: 
  MorseLib();
  void setup();
  char getChar();

private:
  bool _enabled;
};

#endif // MORSE_LIB_H
