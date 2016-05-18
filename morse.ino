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
char password[cursorLimit + 2] = "";

// Global objects
MorseLib ml(PIN_MORSE, PIN_SPEAKER, true);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // set the LCD address to 0x20 for a 16 chars and 2 line display
Bounce db = Bounce(PIN_CLEAR, 20); 

// TODO:
//  check for proper password 'polo'
//    if correct
//      buzzer happy beep
//      set high line on relay to open magnet
//  TODO: put back full timeout (30s)
//  TODO: put back full character limit (15)

void beep(unsigned char speakerPin, int frequencyInHertz, long timeInMilliseconds)     // the sound producing function  
{
  int x;   
  long delayAmount = (long)(1000000/frequencyInHertz); 
  long loopTime = (long)((timeInMilliseconds*1000)/(delayAmount*2)); 
  for (x=0;x<loopTime;x++)   
  {   
      digitalWrite(speakerPin,HIGH); 
      delayMicroseconds(delayAmount); 
      digitalWrite(speakerPin,LOW); 
      delayMicroseconds(delayAmount); 
  }
}

void successSound() {
  beep(PIN_SPEAKER,1200,100);
  delay(80);
  beep(PIN_SPEAKER,1500,100);
  delay(80);
}

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

void resetPassword() {
  cursorPos = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  memset(password, 0, sizeof(password));
}

void loop()
{
  // debounce clear button
  db.update();

  // handle clear button pressed
  if (db.rose()) {
    resetPassword();
  }

  // handle timeout on lcd
  if (millis() - lcdTimeOn > lcdTimeSleep) {
    resetPassword();
    lcd.noBacklight();
  }

  // handle morse code key entered
  char morseChar = ml.getChar(); 
  if (morseChar != '\0') {
    
    // print character to console
    Serial.print(morseChar);

    // update string password
    password[cursorPos] = morseChar;

    // turn on backlight in case its off
    lcdTimeOn = millis();
    lcd.backlight();

    // print character to lcd
    lcd.print(morseChar);

    // update cursor position
    cursorPos++;

    // limit the total displayed to lcd
    if (cursorPos > cursorLimit) {
        cursorPos = cursorLimit;
        lcd.setCursor(cursorLimit, 0);
    } 
  }
}

