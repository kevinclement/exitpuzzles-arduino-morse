#include <LiquidCrystal_I2C.h>
#include <Bounce2.h>
#include "MorseLib.h"

// PIN setup
#define PIN_MORSE   2 // The Morse keyer button
#define PIN_CLEAR   3 // Button to clear lcd by one
#define PIN_SPEAKER 7 // Speaker connection

// Global config
const unsigned long lcdTimeSleep = 4000;   // when to sleep lcd

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

void clearLCD() {
  cursorPos = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
}

// TODO:
//  debug any issues
//  check for proper password 'polo'
//    if correct
//      buzzer happy beep
//      set high line on relay to open magnet
//  after 5 minutes clear screen
//    when wake up should be reset
//  clear is messing with cursor
//  add reset like password when puzzle is over
//  TODO: put back full timeout (30s)
//  TODO: put back full character limit (15)

bool pressed = false;
void loop()
{
  // debounce clear button
  db.update();

  // handle clear button pressed
  if (db.rose()) {
    clearLCD();
  }

  // handle timeout on lcd
  if (millis() - lcdTimeOn > lcdTimeSleep) {
    clearLCD();
    lcd.noBacklight();
  }

  // handle morse code key entered
  char morseChar = ml.getChar(); 
  if (morseChar != '\0') {
    
    // print character to console
    Serial.print(morseChar);

    // turn on backlight in case its off
    lcdTimeOn = millis();
    lcd.backlight();

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

