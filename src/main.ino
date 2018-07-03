#include <LiquidCrystal.h>
#include <Keypad.h>
#include "FastLED.h"

// Initialize LCD display
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// Initialize keypad
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {3, 15, 14, 5}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {4, 2, 6}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// Initialize LEDs
#define DATA_PIN    17 // A3
#define LED_TYPE    WS2813
#define COLOR_ORDER GRB
#define NUM_LEDS    2
#define BRIGHTNESS  255
CRGB leds[NUM_LEDS];

// Cursor position
int cursorLine = 0;
int cursorPos = 0;

// Buzzer
const int buzzer = 16;
int buzzerStatus = 0;

// Configuration
int armTime = 0; // Seconds
int disarmTime = 0; // Seconds
int explodeTime = 0; // Seconds
int disarmTries = 0;
String disarmCode = "";

// === SCREENS ===
//  1: Game select
//  2: Setup - Arm time
//  3: Setup - Disarm time
//  4: Setup - Explode time
//  5: Setup - Code
//  6: Setup - Tries
//  7: Game started: hold button to arm
//  8: Bomb armed: show message
//  9: Bomb armed: write code (show time left)
// 10: Bomb armed, code OK: show message
// 11: Bomb armed, code OK: hold button to disarm (show time left)
// 12: Bomb exploded (time elapsed or wrong code)
// 13: Bomb disarmed
// 14: Wrong code

int currentScreen = 1;
int gameMode = 0;
int bombStatus = 0; // 0: neutral, 1: armed, 2: defused, 3: exploded
int bombTimeLeft = 0;
double bombEnabledSince = 0;

String capturedValue = "";

bool bombIsArmed = false;
bool buttonIsPressed = false;
double pressedSince = 0;

double previousMillis = 0;

// t is time in seconds = millis()/1000;
char * timeToString(unsigned long t){
  static char str[12];
  long h = t / 3600;
  t = t % 3600;
  int m = t / 60;
  int s = t % 60;
  sprintf(str, "%02ld:%02d:%02d", h, m, s);
  return str;
}

void setup() {
  // Serial.begin(9600);

  pinMode(buzzer, OUTPUT);

  keypad.addEventListener(keypadEvent);
  keypad.setHoldTime(2000); // Default is 1000ms
  
  lcd.begin(16, 2); // set up the LCD's number of columns and rows

  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  leds[0] = CRGB::Green;
  leds[1] = CRGB::Green;
  FastLED.show();
  
  drawScreen(1);
}

void loop() {
  keypad.getKey(); // Poll keypad
  doBuzzerStuff();

  if(bombIsArmed){
    bombTimeLeft = explodeTime - ((millis() - bombEnabledSince) / 1000);

    if(bombTimeLeft <= 20){
      buzzerStatus = 3; // About to explode
    }
    
    if(bombTimeLeft <= 0){
      buzzerStatus = 4; // Bomb exploded
      if(currentScreen != 12)
        drawScreen(12);
    }

    if(currentScreen == 9 || currentScreen == 11){
      if(millis() - previousMillis >= 1000){
        lcd.setCursor(0, 0);
        lcd.print(timeToString(bombTimeLeft));
        lcd.setCursor(6 + capturedValue.length(), 1);
        previousMillis = millis();
      }
    }
  }

  if(buttonIsPressed){
    if(bombIsArmed){
      float timeLeft = disarmTime - ((millis() - pressedSince) / 1000);
      lcd.setCursor(12, 1);
      lcd.print(timeLeft);
      
      if(timeLeft <= 0){
        buttonIsPressed = false;
        bombIsArmed = false;
        drawScreen(13); // Bomb disarmed
        buzzerStatus = 5;
      }
    } else {
      float timeLeft = armTime - ((millis() - pressedSince) / 1000);
      lcd.setCursor(12, 0);
      lcd.print(timeLeft);
      
      if(timeLeft <= 0){
        buttonIsPressed = false;
        bombIsArmed = true;
        bombEnabledSince = millis();
        drawScreen(8); // Bomb armed
        buzzerStatus = 2;
      }
    }
  }
}

void keypadEvent(KeypadEvent key){
    switch (keypad.getState()){
      case PRESSED:
        // Serial.print("Pressed: ");
        // Serial.println(key);
        // Serial.print("Current screen: ");
        // Serial.println(currentScreen);
        
        switch (currentScreen){
          case 1: // Main Menu
            captureGameMode(key);
            break;

          case 2: // Set arm time
          case 3: // Set disarm time
          case 4: // Set explode time
          case 5: // Set code
          case 6: // Set tries
          case 9: // Read code
            captureValue(key);
            break;

          case 7:
          case 11:
            if(key == '#'){
              pressedSince = millis();
              buttonIsPressed = true;
              buzzerStatus = 1;

              if(bombIsArmed){
                lcd.clear();
                lcd.setCursor(0, 1);
                lcd.print("DEFUSING:");
              } else {
                lcd.clear();
                lcd.print("ARMING:");
              }

              previousMillis = 0; // Update clock
            }
            break;
        
          default:
            break;
        }
        break;

      case RELEASED:
        if(buttonIsPressed){
          if(currentScreen == 7 || currentScreen == 11){
            if(key == '#'){
              buttonIsPressed = false;
              pressedSince = 0;
              previousMillis = 0; // Update clock
              // Serial.print("Button was pressed for: ");
              // Serial.print((millis() - pressedSince) / 1000);
              // Serial.println(" seconds");

              // Not enough time
              if(bombIsArmed){
                drawScreen(11);
                buzzerStatus = 2;
              } else {
                drawScreen(7);
                buzzerStatus = 0;
              }
            }
          }
        }
        break;

      case HOLD:
        // If bomb exploded or defused, reset the Arduino
        if(currentScreen == 12 || currentScreen == 13){
          if(key == '#'){
            softReset();
          }
        }
        break;
    }
}

void captureGameMode(char key){
  String modeName = "";

  switch (key){
    case '#':
      if(gameMode != 0){
        beep(100);
        drawScreen(2);
      } else {
        beepError();
      }
      break;
    case '1':
      modeName = "1. Button only   ";
      gameMode = 1;
      beep(100);
      break;
    case '2':
      modeName = "2. Code + button";
      gameMode = 2;
      beep(100);
      break;
    case '3':
      modeName = "3. Code only    ";
      gameMode = 3;
      beep(100);
      break;
  
    default:
      modeName = "                ";
      gameMode = 0;
      beepError();
      break;
  }

  lcd.setCursor(0, 1);
  lcd.print(modeName);
}

void beepError(){
  beep(30);
  delay(30);
  beep(30);
  delay(30);
  beep(30);
  delay(30);
  beep(30);
}

void drawScreen(int screenNumber){
  lcd.clear();
  currentScreen = screenNumber;

  switch (screenNumber){    
    case 1:
      // Draw main menu
      lcd.print("Select mode 1-3:");
      break;

    case 2:
      // Draw arm time setup
      lcd.print("Arm time (s):");
      lcd.setCursor(0, 1);
      lcd.blink();
      break;

    case 3:
      // Draw disarm time setup
      lcd.print("Defuse time (s):");
      lcd.setCursor(0, 1);
      lcd.blink();
      break;

    case 4:
      // Draw explode time setup
      lcd.print("Expl. time (s):");
      lcd.setCursor(0, 1);
      lcd.blink();
      break;

    case 5:
      // Draw disarm code setup
      lcd.print("Defuse code:");
      lcd.setCursor(0, 1);
      lcd.blink();
      break;

    case 6:
      // Draw tries setup
      lcd.print("Defuse tries:");
      lcd.setCursor(0, 1);
      lcd.blink();
      break;

    case 7:
      // Game start
      lcd.noBlink();
      lcd.print("== BOMB READY ==");
      lcd.setCursor(0, 1);
      lcd.print("Hold # to arm");
      break;

    case 8:
      // Bomb armed message
      lcd.print("BOMB ARMED!");
      lcd.setCursor(0, 1);
      lcd.print("Go get cover");
      beepArmDisarm();
      drawScreen(9);
      break;

    case 9:
      // Bomb armed, enter code
      // lcd.noBlink();
      lcd.setCursor(0, 1);
      lcd.print("Code: ");
      lcd.blink();
      break;

    case 10:
      // Code OK
      lcd.noBlink();
      lcd.print("CODE OK");
      beep(800);
      drawScreen(11);
      break;

    case 11:
      // Hold to defuse
      lcd.noBlink();
      lcd.setCursor(0, 1);
      lcd.print("Hold # to defuse");
      break;

    case 12:
      // Bomb exploded
      lcd.noBlink();
      lcd.print("BOMB EXPLODED!!");
      lcd.setCursor(0, 1);
      lcd.print("Terrorists win");
      break;

    case 13:
      // Bomb defused
      lcd.noBlink();
      lcd.print("BOMB DEFUSED!!");
      lcd.setCursor(0, 1);
      lcd.print("Counter ter. win");
      break;

    case 14:
      // Wrong code
      lcd.noBlink();
      lcd.print("WRONG CODE");
      lcd.setCursor(0, 1);
      lcd.print("Tries left: ");
      lcd.print(disarmTries);
      beepError();
      delay(1000);
      drawScreen(9);
      break;

    default:
      break;
  }
}

void captureValue(char key){
  switch (key){
    case '*':
      beepError();
      clearCode();
      break;

    case '#':
      beep(100);
      if(capturedValue != ""){
        nextSetupStep();
      } else {
        beepError();
      }
      break;

    default:
      beep(100);
      capturedValue += key;
      lcd.print(key);
      
      // Serial.println(capturedValue);
      break;
  }
}

void nextSetupStep(){
  switch (currentScreen){
    case 2:
      armTime = capturedValue.toInt();
      capturedValue = "";
      // Serial.print("Arm time set: ");
      // Serial.println(armTime);
      drawScreen(3);
      break;

    case 3:
      disarmTime = capturedValue.toInt();
      capturedValue = "";
      // Serial.print("Disarm time set: ");
      // Serial.println(disarmTime);
      drawScreen(4);
      break;

    case 4:
      explodeTime = capturedValue.toInt();
      capturedValue = "";
      // Serial.print("Explode time set: ");
      // Serial.println(explodeTime);
      drawScreen(5);
      break;

    case 5:
      disarmCode = capturedValue;
      capturedValue = "";
      // Serial.print("Code set: ");
      // Serial.println(disarmCode);
      drawScreen(6);
      break;

    case 6:
      disarmTries = capturedValue.toInt();
      capturedValue = "";
      // Serial.print("Disarm tries set: ");
      // Serial.println(disarmTries);
      drawScreen(7);
      break;

    case 9:
      if(disarmCode == capturedValue){
        // Code OK
        drawScreen(10);
      } else {
        // Wrong code
        disarmTries--;

        if(disarmTries <= 0){
          // Bomb explodes
          drawScreen(12);
          buzzerStatus = 4;
        } else {
          drawScreen(14);
        }
      }

      capturedValue = "";
      break;
  
    default:
      beepError();
      break;
  }
}

void clearCode(){
  capturedValue = "";
  if(currentScreen == 9){
    lcd.setCursor(6, 1);
    lcd.print("          ");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("                ");
  }
}

void softReset(){
  asm volatile ("  jmp 0");
}