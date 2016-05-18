#include <LiquidCrystal_I2C.h>
#include <Bounce2.h>
#include "MorseLib.h"

// PIN setup
#define PIN_MORSE   2 // The Morse keyer button
#define PIN_CLEAR   3 // Button to clear lcd by one
#define PIN_SPEAKER 7 // Speaker connection
#define PIN_RELAY   8 // Relay pin

// CONST
#define PASSWORD "EE"
#define LCD_SLEEP 10000 // when to sleep lcd
#define RELAY_ON 0
#define RELAY_OFF 1

// Global vars
unsigned long lcdTimeOn = 0; // last time we keyed
int cursorPos = 0;           // current cursor position
const int cursorLimit = 3;   // limit number of characters to display
char password[cursorLimit + 2] = "";
bool enabled = true;
bool magnetOn = true;

// Global objects
MorseLib ml(PIN_MORSE, PIN_SPEAKER, true);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // set the LCD address to 0x20 for a 16 chars and 2 line display
Bounce db = Bounce(PIN_CLEAR, 20); 

// TODO:
//  TODO: put back full timeout (5 minutes)
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

void winner() {
  Serial.println("");
  Serial.println("WINNER!!");
  successSound();
  enabled = false;
  magnetOn = false;
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Morse decoder by kevinc");

  // setup morse button
  ml.setup();

  // setup clear button
  pinMode(PIN_CLEAR, INPUT_PULLUP);

  // setup relay
  digitalWrite(PIN_RELAY, RELAY_OFF);
  pinMode(PIN_RELAY, OUTPUT);
    
  // setup the lcd
  lcd.init();
}

void reset() {
  cursorPos = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  memset(password, 0, sizeof(password));
  enabled = true;
  magnetOn = true;
}

void loop()
{
  // write out to the relay
  digitalWrite(PIN_RELAY, magnetOn ? RELAY_ON : RELAY_OFF);
  
  // handle timeout on lcd
  if (millis() - lcdTimeOn > LCD_SLEEP) {
    reset();
    lcd.noBacklight();
    lcd.noCursor();
  }

  // don't do work if we won
  if (!enabled) {
    return;
  }
  
  // debounce clear button
  db.update();

  // handle clear button pressed
  if (db.rose()) {
    reset();
  }

  // handle morse code key entered
  char morseChar = ml.getChar(); 
  if (morseChar != '\0') {
    
    // print character to console
    Serial.print(morseChar);

    // turn on backlight in case its off
    lcdTimeOn = millis();
    lcd.backlight();
    lcd.cursor();

    // print character to lcd
    lcd.print(morseChar);

    // update string password
    password[cursorPos] = morseChar;

    // check for win condition
    if (String(password) == PASSWORD) {
      winner();
    }
    else {
      // update cursor position
      cursorPos++;
  
      // limit the total displayed to lcd
      if (cursorPos > cursorLimit) {
          cursorPos = cursorLimit;
          lcd.setCursor(cursorLimit, 0);
      } 
    }
  }
}

