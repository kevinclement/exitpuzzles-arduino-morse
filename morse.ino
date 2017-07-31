#include <LiquidCrystal_I2C.h>
#include <Bounce2.h>
#include "MorseLib.h"
#include <SoftwareSerial.h>

// PIN setup
#define PIN_MORSE   2 // The Morse keyer button
#define PIN_CLEAR   3 // Button to clear lcd by one
#define PIN_SPEAKER 7 // Speaker connection
#define PIN_RELAY   8 // Relay pin

// CONST
#define PASSWORD "POLO"
#define PASSWORD_TYPO "POLLO"
#define ALMOST "MARCO"
#define ALMOST_TYPO "MARKO"
#define LCD_SLEEP 300000 // when to sleep lcd (5minutes)
//#define LCD_SLEEP 10000 
#define LCD_CHAR_LIMIT 10
#define RESET_TIME 10000 // how long to hold button before a reset
#define DISPLAY_PREFIX "CODE:"
#define FEEDBACK_LINE 1 // what line is feedback on
#define DISPLAY_LINE 0  // what line is display on
#define RELAY_ON 0
#define RELAY_OFF 1

// Global vars
unsigned long lcdTimeOn = 0; // last time we keyed
unsigned long buttonHeld = 0;
int cursorPos = 0;           // current cursor position
char password[LCD_CHAR_LIMIT + 2] = "";
bool enabled = true;
bool magnetOn = true;
int dotDashCount = 0;
bool almostModeOn = false;
bool clueModeOn = false;

// Global objects
MorseLib ml(PIN_MORSE, PIN_SPEAKER, true);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // set the LCD address to 0x20 for a 16 chars and 2 line display
Bounce db = Bounce(PIN_CLEAR, 20);
SoftwareSerial bluetooth(10, 9); // RX, TX  

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

void shortBeep(int howLong) {
  digitalWrite(PIN_SPEAKER,HIGH); 
  delay(howLong);
  digitalWrite(PIN_SPEAKER,LOW);  
}

void successSound() {
  beep(PIN_SPEAKER,1200,100);
  delay(80);
  beep(PIN_SPEAKER,1200,100);
  beep(PIN_SPEAKER,1500,200);
}

void clueSound() {
  shortBeep(200);
  delay(50);
  shortBeep(100);
  delay(35);
  shortBeep(100);
  delay(35);
  shortBeep(200);
  delay(50);
  shortBeep(200);
  delay(50);
  shortBeep(100);
  delay(35);
  shortBeep(100);
  delay(35);
  shortBeep(200);    
}

void feedbackSound() {
  beep(PIN_SPEAKER,415,100);
  delay(80);
  beep(PIN_SPEAKER,279,100);
  delay(80);
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Morse decoder by kevinc");

  // setup bluetooth serial connection
  bluetooth.begin(9600);

  // setup morse button
  ml.setup();

  // setup clear button
  pinMode(PIN_CLEAR, INPUT_PULLUP);

  // setup relay
  digitalWrite(PIN_RELAY, RELAY_OFF);
  pinMode(PIN_RELAY, OUTPUT);
    
  // setup the lcd
  lcd.init();
  lcd.setCursor(0, DISPLAY_LINE);
  lcd.print(DISPLAY_PREFIX); 
}

void clearFeedback() {
  dotDashCount = 0;
  lcd.setCursor(0, FEEDBACK_LINE);
  lcd.print("                ");
}

void clearPassword() {
  cursorPos = 0;
  lcd.setCursor(0, DISPLAY_LINE);
  lcd.print("                ");
  lcd.setCursor(0, DISPLAY_LINE);
  lcd.print(DISPLAY_PREFIX);
  memset(password, 0, sizeof(password));
}

void winner() {
  Serial.println("");
  Serial.println("WINNER!!");

  resetScreen();
  lcd.setCursor(0, 0);
  lcd.print("DRAWER UNLOCKED!");
  lcd.noCursor();
  
  successSound();
  enabled = false;
  magnetOn = false;
}

void almostAWinner() {
  beep(PIN_SPEAKER,415,100);
  delay(80);
  beep(PIN_SPEAKER,279,100);
  delay(80);
  
  resetScreen();
  lcd.setCursor(0, 0);
  lcd.print("    Almost!     ");
  lcd.setCursor(0, 1);
  lcd.print("Try the response");
  lcd.noCursor();
  almostModeOn = true;
}

void resetScreen() {
  clearFeedback();
  clearPassword();
}

void reset() {
  resetScreen();
  enabled = true;
  magnetOn = true;
  almostModeOn = false;
  clueModeOn = false;
}

void timeout() {
  if (enabled) {
    resetScreen();
  } else {
    lcd.clear();
  }
  lcd.noBacklight();
  lcd.noCursor();
}

void eraseALetter() {
  clearFeedback();

  if (almostModeOn==true || cursorPos >= LCD_CHAR_LIMIT) {
    reset();
  }
  else {
    cursorPos--;

    // dont allow cursor to move back past initial display
    if (cursorPos < 0) {
      cursorPos = 0;
    }

    // clear the stored password and set the cursor to clear single character
    password[cursorPos] = '\0';

    lcd.setCursor(strlen(DISPLAY_PREFIX) + cursorPos, DISPLAY_LINE);
    lcd.print(' ');
    lcd.setCursor(strlen(DISPLAY_PREFIX) + cursorPos, DISPLAY_LINE);
  }
}

void readAnyBluetoothMessage() {
  if (bluetooth.available()) {
    String str = bluetooth.readStringUntil('\n');
    Serial.print("got in: ");
    Serial.println(str);

    // ignore connection messages
    if (str.length() < 3) {
      return;
    }
    
    clueModeOn = true;

    String line1;
    String line2;
    
    resetScreen();
    lcdTimeOn = millis();
    lcd.backlight();

    lcd.setCursor(0, 0);

    bool feedback = str.startsWith("@");     // starts with @ means its a feedback one, not a clue, change sound
    if (feedback) {
      str = str.substring(1);
    }
    int newLineCharIndex = str.indexOf('#'); // # means newline since I only send 1 thing at a time from app

    if (newLineCharIndex > 0) {
      line1 = str.substring(0, newLineCharIndex);
      line2 = str.substring(newLineCharIndex + 1);  
    }
    else {
      line1 = str;
    }

    // pad string if needed
    int l1Length = line1.length();
    if (line1.length() < 16) {
      for(int i=line1.length(); i<16; i++) {
        line1 += " ";
      }
    }
    if (line2.length() < 16) {
      for(int i=line2.length(); i<16; i++) {
        line2 += " ";
      }
    }

    Serial.println("Setting clue to: ");
    Serial.println(line1);
    Serial.println(line2);

    // set lcd on screen
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
    lcd.noCursor();

    // make sound
    if (feedback) {
      feedbackSound();
    } else {
      clueSound();
    }
  }
}

void loop()
{
  // write out to the relay
  digitalWrite(PIN_RELAY, magnetOn ? RELAY_ON : RELAY_OFF);

  // read bluetooth ble connection
  readAnyBluetoothMessage();
  
  // handle timeout on lcd
  if (millis() - lcdTimeOn > LCD_SLEEP) {
    timeout();
  }

  // debounce clear button
  db.update();

  // record clear button down
  if (db.fell()) {
    buttonHeld = millis();
  }

  // reset the length of the button press when its unpressed
  if (db.rose()) {
    buttonHeld = 0;
  }

  // if we've been holding the button down long enough, reset the whole thing
  if (db.read() == 0 && buttonHeld > 0 && millis() - buttonHeld > RESET_TIME) {
    Serial.println("Resetting machine.");
    buttonHeld = 0;
    reset();
  }

  // don't do work if we won and not a reset button
  if (!enabled) {
    return;
  }

  // if a clue has come in and then they pressed reset, then we should fully reset it all
  if (clueModeOn && db.rose()) {
    Serial.println("Clue mode is enabled, and button pressed.  Resetting display.");
    resetScreen();
    clueModeOn = false;
    ml.reset();
    delay(100);
    return;
  }

  // handle clear button pressed
  if (db.rose()) {
    eraseALetter();
  }

  // handle morse code key entered
  char morseChar = ml.getChar(); 
  
  // if they almost got it but then used the handle, reset it all
  if (almostModeOn==true && (morseChar == '.' || morseChar == '-')) {
    reset();
  }
  else if (morseChar == '.' || morseChar == '-') {
    // print to lcd dots and dashes to help users determine what they were doing
    lcd.setCursor(dotDashCount, FEEDBACK_LINE);
    lcd.print(morseChar);
    dotDashCount++;
    lcd.setCursor(strlen(DISPLAY_PREFIX) + cursorPos, DISPLAY_LINE);    

    // turn on backlight in case its off
    lcdTimeOn = millis();
    lcd.backlight();
    lcd.cursor();

  } else if (morseChar != '\0') {
    clearFeedback();
    lcd.setCursor(strlen(DISPLAY_PREFIX) + cursorPos, DISPLAY_LINE);
    
    // print character to console
    Serial.print(morseChar);

    // print character to lcd
    lcd.print(morseChar);

    // update string password
    password[cursorPos] = morseChar;

    // check for win condition
    if (String(password) == PASSWORD || String(password) == PASSWORD_TYPO) {
      winner();
    }
    else if (String(password) == ALMOST || String(password) == ALMOST_TYPO) {
      almostAWinner();
    } else {
      // update cursor position
      cursorPos++;
  
      // limit the total displayed to lcd
      if (cursorPos > LCD_CHAR_LIMIT) {
          cursorPos = LCD_CHAR_LIMIT;
          lcd.setCursor(strlen(DISPLAY_PREFIX) + LCD_CHAR_LIMIT, DISPLAY_LINE);
      } 
    }
  }
}
