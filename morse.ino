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
unsigned long lcdTimeOn = 0;   // last time we keyed
int charCount = 0;
int charLimit = 15;
boolean lcdLightOn = false;

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
//  add back lcd limit logic
//  cleanup button press debounce
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

bool pressed = false;
void loop()
{

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
   
  // debounce clear button
  db.update();

  // handle clear button pressed
  if (db.rose()) {
    
  }

  // handle timeout on lcd
  if (millis() - lcdTimeOn > lcdTimeSleep) {
    lcdLightOn = false;
    lcd.clear();
    lcd.noBacklight();
  }

  // handle morse code key entered
  char morseChar = ml.getChar(); 
  if (morseChar != '\0') {
    
    // Echo to console
    Serial.print(morseChar);

    // TODO: handle morse character
  }
}

