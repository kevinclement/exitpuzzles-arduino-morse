#include <LiquidCrystal_I2C.h>
#include <InputDebounce.h>

// the debounce time. Keep well below dotTime!!
#define BUTTON_DEBOUNCE_DELAY   20   // [ms]
unsigned long debounceDelay = 20;

// Other Morse variables
unsigned long dotTime = 75;   // morse dot time length in ms
unsigned long dashTime = 300;
unsigned long wordSpace = 20000;

const int morseInPin = 2;      // The Morse keyer button
const int clearButtonPin = 3;
const int morseOutPin =  7;    // For Morse code output
unsigned long markTime = 0;    // timers for mark and space in morse signal
unsigned long spaceTime = 0;   // E=MC^2 ;p
unsigned long lcdTimeOn = 0;   // last time we keyed
unsigned long lcdTimeSleep = 300000;   // when to sleep lcd
boolean morseSpace = false;    // Flag to prevent multiple received spaces
boolean gotLastSig = true;     // Flag that the last received morse signal is decoded as dot or dash
int charCount = 0;
int charLimit = 15;

const int morseTreetop = 31;   // character position of the binary morse tree top.
const int morseTableLength = (morseTreetop * 2) + 1;
const int morseTreeLevels = log(morseTreetop + 1) / log(2);
int morseTableJumper = (morseTreetop + 1) / 2;
int morseTablePointer = morseTreetop;

// Morse code binary tree table (or, dichotomic search table)
char morseTable[] = "5H4S?V3I?F?U??2E?L?R???A?P?W?J1 6B?D?X?N?C?K?Y?T7Z?G?Q?M8??O9?0";
//char morseTable[] = "?H?S?B?I?L?D?Z?E?F?R?C?N?P?G?J? ?V?U?X?A?C?K?Q?T?Z?W?Y?M*J?O*??";

int morseSignals;              // nr of morse signals to send in one morse character
char morseSignal[] = "......"; // temporary string to hold one morse character's signals to send
int morseSignalPos = 0;
int sendingMorseSignalNr = 0;
unsigned long sendMorseTimer = 0;

boolean morseEcho = true; // Echoes character to encode back to serial and Morse signal input to output pin
boolean sendingMorse = false;
boolean morseSignalState = false;
boolean lastKeyerState = false;
boolean lastClearButton = false;
boolean clearButtonState = false;
boolean lcdLightOn = false;


unsigned long lastDebounceTime = 0;  // the last time the input pin was toggled
unsigned long lastDebounceButton = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2);  // set the LCD address to 0x20 for a 16 chars and 2 line display
static InputDebounce buttonTest; // not enabled yet, setup has to be called later
static unsigned int buttonTest_OnTimeLast = 0;

void setup()
{
  pinMode(morseInPin, INPUT);
  pinMode(clearButtonPin, INPUT_PULLUP);
  digitalWrite(morseInPin, HIGH); // internal pullup resistor on
  pinMode(morseOutPin, OUTPUT);
  Serial.begin(9600);
  Serial.println("Morse decoder by kevinc");
  markTime = millis();
  spaceTime = millis();

  // initialize the lcd
  lcd.init();
  lcd.cursor();

  buttonTest.setup(clearButtonPin, DEFAULT_INPUT_DEBOUNCE_DELAY);
}

// TODO:
//  morse to library to clean this up
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

void loop()
{

  // char morseChar = morse.getChar();
  // if (morseChar != '')
  // {
  //

  unsigned int buttonTest_OnTime = buttonTest.process(millis());
  if (buttonTest_OnTime) {
    buttonTest_OnTimeLast = buttonTest_OnTime;
  }
  else if (buttonTest_OnTimeLast) {
    Serial.print("YES!!!");
    Serial.println(millis());
    buttonTest_OnTimeLast = 0;
  }

  if (millis() - lcdTimeOn > lcdTimeSleep) {
    lcdLightOn = false;
    lcd.clear();
    lcd.noBacklight();
  }

  boolean morseKeyer = !digitalRead(morseInPin); // inverted for active-low input

  // If the switch changed, due to noise or pressing:
  if (morseKeyer != lastKeyerState)
  {
    lastDebounceTime = millis(); // reset timer
  }

  // debounce the morse keyer
  if ((millis() - lastDebounceTime) > debounceDelay)
  {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
    morseSignalState = morseKeyer;
    // differentiante mark and space times
    if (morseSignalState) markTime = lastDebounceTime; else spaceTime = lastDebounceTime;
  }

  // Morse output, or a feedback when keying.
  if (!sendingMorse && morseEcho) {
    digitalWrite(morseOutPin, morseSignalState);
  }

  // Send Morse signals to output
  if (sendingMorse)
  {
    switch (morseSignal[sendingMorseSignalNr])
    {
      case '.': // Send a dot (actually, stop sending a signal after a "dot time")
        if (millis() - sendMorseTimer >= dotTime)
        {
          digitalWrite(morseOutPin, LOW);
          sendMorseTimer = millis();
          morseSignal[sendingMorseSignalNr] = 'x'; // Mark the signal as sent
        }
        break;
      case '-': // Send a dash (same here, stop sending after a dash worth of time)
        if (millis() - sendMorseTimer >= dashTime)
        {
          digitalWrite(morseOutPin, LOW);
          sendMorseTimer = millis();
          morseSignal[sendingMorseSignalNr] = 'x'; // Mark the signal as sent
        }
        break;
      case 'x': // To make sure there is a pause between signals and letters
        if (sendingMorseSignalNr < morseSignals - 1)
        {
          // Pause between signals in the same letter
          if (millis() - sendMorseTimer >= dotTime)
          {
            sendingMorseSignalNr++;
            digitalWrite(morseOutPin, HIGH); // Start sending the next signal
            sendMorseTimer = millis();       // reset the timer
          }
        } else {
          // Pause between letters
          if (millis() - sendMorseTimer >= dashTime)
          {
            sendingMorseSignalNr++;
            sendMorseTimer = millis();       // reset the timer
          }
        }
        break;
      case ' ': // Pause between words (minus pause between letters - already sent)
      default:  // Just in case its something else
        if (millis() - sendMorseTimer > wordSpace - dashTime) sendingMorse = false;
    }
    if (sendingMorseSignalNr >= morseSignals) sendingMorse = false; // Ready to encode more letters
  }

  // Decode morse code
  if (!morseSignalState)
  {
    if (!gotLastSig)
    {
      if (!lcdLightOn) {
        lcd.backlight();
        lcdLightOn = true;
      }

      lcdTimeOn = millis();
      if (morseTableJumper > 0)
      {
        // if pause for more than half a dot, get what kind of signal pulse (dot/dash) received last
        if (millis() - spaceTime > dotTime / 2)
        {
          // if signal for more than 1/4 dotTime, take it as a valid morse pulse
          if (spaceTime - markTime > dotTime / 4)
          {
            // if signal for less than half a dash, take it as a dot, else if not, take it as a dash
            // (dashes can be really really long...)
            if (spaceTime - markTime < dashTime / 2) morseTablePointer -= morseTableJumper;
            else morseTablePointer += morseTableJumper;
            morseTableJumper /= 2;
            gotLastSig = true;
          }
        }
      } else { // error if too many pulses in one morse character
        Serial.println("<ERROR: unrecognized signal!>");
        gotLastSig = true;
        morseTableJumper = (morseTreetop + 1) / 2;
        morseTablePointer = morseTreetop;
      }
    }
    // Write out the character if pause is longer than 2/3 dash time (2 dots) and a character received
    //if ((millis()-spaceTime >= (dotTime*2)) && (morseTableJumper < 16))
    if ((millis() - spaceTime >= 1000) && (morseTableJumper < 16))
    {
      char morseChar = morseTable[morseTablePointer];
      Serial.print(morseChar);
      lcd.print(morseChar);

      charCount++;
      if (charCount > charLimit) {
        lcd.setCursor(charLimit, 0);
      }

      morseTableJumper = (morseTreetop + 1) / 2;
      morseTablePointer = morseTreetop;
    }

    // Write a space if pause is longer than 2/3rd wordspace
    if (millis() - spaceTime > (wordSpace * 2 / 3) && morseSpace == false)
    {
      Serial.print(" ");
      morseSpace = true ; // space written-flag
    }

  } else {
    // while there is a signal, reset some flags
    gotLastSig = false;
    morseSpace = false;
  }

  // save last state of the morse signal for debouncing
  lastKeyerState = morseKeyer;
}

