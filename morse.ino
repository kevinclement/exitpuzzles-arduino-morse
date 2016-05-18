#include <LiquidCrystal_I2C.h>
#include <Bounce2.h>
#include "MorseLib.h"

// PIN setup
#define PIN_MORSE   2 // The Morse keyer button
#define PIN_CLEAR   3 // Button to clear lcd by one
#define PIN_SPEAKER 7 // Speaker connection

// Global config
unsigned long lcdTimeSleep = 300000;   // when to sleep lcd

// Global vars
unsigned long lcdTimeOn = 0; // last time we keyed
int cursorPos = 0;           // current cursor position
const int cursorLimit = 3;   // limit number of characters to display

// Global objects
MorseLib ml(PIN_MORSE, PIN_SPEAKER, true);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // set the LCD address to 0x20 for a 16 chars and 2 line display
Bounce db = Bounce(PIN_CLEAR, 20); 

void setup()
{
  Serial.begin(9600);
  Serial.println("Morse decoder by kevinc");

  // setup morse button
  ml.setup();

  // setup clear button
  pinMode(PIN_CLEAR, INPUT_PULLUP);
    
  // setup the lcd
  lcd.init();
  lcd.cursor();
}

// TODO:
//  add back lcd light logic
//  debug any issues
//  check for proper password 'polo'
//    if correct
//      buzzer happy beep
//      set high line on relay to open magnet
//  after 5 minutes clear screen
//    when wake up should be reset
//  clear is messing with cursor
//  add reset like password when puzzle is over

bool pressed = false;
void loop()
{

  // TODO: ADD BACK
  // if (!lcdLightOn) {
  //      lcd.backlight();
  //      lcdLightOn = true;
  //  }

  // debounce clear button
  db.update();

  // handle clear button pressed
  if (db.rose()) {
    cursorPos = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
  }

  // handle timeout on lcd
  if (millis() - lcdTimeOn > lcdTimeSleep) {
    lcd.clear();
    lcd.noBacklight();
  }

  // handle morse code key entered
  char morseChar = ml.getChar(); 
  if (morseChar != '\0') {
    
    // print character to console
    Serial.print(morseChar);

    // print character to lcd
    lcd.print(morseChar);

    // TODO: update string representation

    // update cursor position
    cursorPos++;

    // limit the total displayed to lcd
    if (cursorPos > cursorLimit) {
        cursorPos = cursorLimit;
        lcd.setCursor(cursorLimit, 0);
    } 
  }
}

