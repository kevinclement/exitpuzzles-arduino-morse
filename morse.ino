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

boolean lcdLightOn = false;

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
  if (morseChar != '\0') {
    
    // Echo to console
    Serial.print(morseChar);

    // TODO: handle morse character
  }
}

