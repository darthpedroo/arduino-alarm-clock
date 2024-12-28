#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>


// Pin configuration for DS1302 RTC module
const int RTC_CLK = 7;  // Clock Pin (SCLK)
const int RTC_DAT = 8;  // Data Pin (I/O)
const int RTC_RST = 6;  // Chip Enable Pin (CE)

// Create an instance of the DS1302 RTC
ThreeWire myWire(RTC_DAT, RTC_CLK, RTC_RST);
RtcDS1302<ThreeWire> Rtc(myWire);

int lastSecond = -1;  // Variable to store the last second checked

// Alarm variables:
int alarmHour = 21;
int alarmMinute = 0;
int alarmSecond = 0;

// Liquid Crystal
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust the address if necessary

// Buzzer
const int buzzerPin = 9;

// Timing variables
unsigned long lastBuzzMillis = 0;   // Last time the buzzer toggled
unsigned long lastPrintMillis = 0; // Last time the LCD was updated
unsigned long buzzInterval = 500;   // Buzzer toggle interval (ms)
const unsigned long printInterval = 1000; // LCD update interval (ms)
bool buzzerState = false;

int beepCount = 0;            // Número de beeps completados
int totalBeeps = 0;           // Número total de beeps que se desean
bool isAlarmActive = false;   // Estado de la alarma

// JOYSTICK 
int xPin = A0;
int yPin = A1;
int buttonPin = 2;

int xVal;
int yVal;
int buttonState;

// LCD Menu
// 0: Normal Clock
// 1: Set alarm
// 2: Wake up alarm

unsigned long lastButtonPress = 0; // Time of the last button press
unsigned long lastScrollJoystick = 0; 
unsigned long debounceDelay = 1500;  // Debounce delay in milliseconds (adjust as needed)
unsigned long debounceDelayScroll = 150;  // Debounce delay in milliseconds (adjust as needed)

int menuValue = 0;
int maxMenuValue = 2;

String phrases[] = {
    "STAY HARD",
    "HAWK",
    "YOU ARE JUST A BITCH"
  };

int randomIndex;         // Declare globally to store random index
String currentPhrase;    // Declare globally to store the chosen phrase


// Snooze 

int currentSnooze = 5;
int snoozeIncremental = 5;

int maxSnooze = 60;

void turnBuzzerOn() {
  digitalWrite(buzzerPin, HIGH); // Set the buzzer pin to HIGH
}

void turnBuzzerOff() {
  digitalWrite(buzzerPin, LOW); // Set the buzzer pin to LOW
}


void handleAlarm(int hour, int minute, int second, RtcDateTime now) {

  if (now.Hour() == hour && now.Minute() == minute && now.Second() == second) {
    Serial.println("ALARM CLOCK TRIGGERED!");
    printAlarmMessageLcd(lcd);

    unsigned long buzzerStartMillis = millis();  // Start the timer
    unsigned long buzzerInterval = 1000;  // 1 second interval
    bool buzzerState = false;  // Track buzzer state (on/off)
    unsigned long lastBuzzerMillis = millis();  // To manage buzzer intervals

    while (millis() - buzzerStartMillis < 5000) {  // Run alarm for 5 seconds
      // Check joystick click
      if (isJoystickClicked(buttonPin)) {
        unsigned long currentMillis = millis();

        lastButtonPress = currentMillis;
        alarmMinute += currentSnooze;
        
        if (alarmMinute >= 60) {
          alarmMinute -= 60;
          alarmHour +=1;
        }

        if (alarmHour >= 24) {
          alarmHour = 0;
        }
        
        menuValue = 0;
        break;  
        
      }

      unsigned long currentMillis = millis();
      if (currentMillis - lastBuzzerMillis >= buzzerInterval) {
        lastBuzzerMillis = currentMillis;  // Update the buzzer timer

        // Toggle buzzer state
        buzzerState = !buzzerState;
        if (buzzerState) {
          turnBuzzerOn();
        } else {
          turnBuzzerOff();
        }
      }
    }
    turnBuzzerOff();  // Make sure to turn off the buzzer after the alarm is finished
  }
}



void triggerBuzzerAlarm(int beeps) {
  totalBeeps = beeps;   // Establecer el número total de beeps
  beepCount = 0;        // Reiniciar el conteo de beeps
  isAlarmActive = true; // Activar la alarma
}

void handleTimePrinting(RtcDateTime now) {
  unsigned long currentMillis = millis();
  if (currentMillis - lastPrintMillis >= printInterval) {
    lastPrintMillis = currentMillis;

    printTime(now);
    printTimeLcd(now, lcd, currentPhrase, 0,0);
  }
}


void handleAlarmSetup(){
    
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastScrollJoystick >= debounceDelayScroll) {

      lastScrollJoystick = currentMillis;
  
      if (isPinHigh(xPin) ) {
        if (alarmHour >= 23) {
          alarmHour = 0;}
        else {
          alarmHour++;
        }
      }
      else if (isPinLow(xPin)) {
        if (alarmHour <= 0){
          alarmHour = 23;
        }
        else{
          alarmHour--;
        }
      }
      else if (isPinLow(yPin)) {
        if ( alarmMinute >= 59) {
          alarmMinute = 0;
        }
        else {
          alarmMinute ++;
        }
      }
      else if (isPinHigh(yPin)) {
        if ( alarmMinute <= 0) {
          alarmMinute = 59;
        }
        else {
          alarmMinute --;
        }

      }

    }
}

void menu(int menuValue, RtcDateTime now) {
  
  if (menuValue == 0){
    handleTimePrinting(now);
    handleAlarm(alarmHour, alarmMinute, alarmSecond, now);
  }
  else if (menuValue == 1){

    printAlarmSetupLcd(lcd,alarmHour,alarmMinute);
    handleAlarmSetup();
  }
  
  else if (menuValue ==2 ){
    String snoozeText = "Snooze Config";

    unsigned long currentMillis = millis();
    
    if (currentMillis - lastScrollJoystick >= debounceDelayScroll) {

      lastScrollJoystick = currentMillis;
      
      if (isPinLow(yPin)) {

        if (currentSnooze >= maxSnooze) {
          currentSnooze = 0;
        }
        else{
          currentSnooze += snoozeIncremental;    
         }        
      }
      else if (isPinHigh(yPin)) {

        if (currentSnooze <= 0){
          currentSnooze = maxSnooze;
        }
        else{
          currentSnooze -= snoozeIncremental;   
        }
      }
      printSnoozeConfig(snoozeText, currentSnooze);
    
    }

    

  }
  
}

void setup() {
  Serial.begin(9600);  // Start serial communication
  Serial.println(randomIndex);
  Serial.println("randomIndex");  
  Rtc.Begin();         // Initialize the RTC

  int numPhrases = sizeof(phrases) / sizeof(phrases[0]);
  randomSeed(analogRead(A3));
  randomIndex = random(0, numPhrases);
  currentPhrase = phrases[randomIndex];

  Serial.println("HAWK TUAH");
  Serial.println(currentPhrase);

  // Set RTC to compile time (if necessary)
  //RtcDateTime currentTime = RtcDateTime(__DATE__, __TIME__);
  //Rtc.SetDateTime(currentTime);

  lcd.init(); // Initialize the LCD
  lcd.backlight(); // Turn on the backlight

  pinMode(buzzerPin, OUTPUT);
  pinMode(xPin, INPUT);
  pinMode(yPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  // Obtener la hora actual
  RtcDateTime now = Rtc.GetDateTime();

  // Manejar tareas asincrónicas

    menu(menuValue, now);

    if (isJoystickClicked(buttonPin)) {
    // Get the current time
    unsigned long currentMillis = millis();
    
    // If enough time has passed since the last press, change menu
    if (currentMillis - lastButtonPress >= debounceDelay) {
      Serial.println("Button pressed");

      // Update last button press time
      lastButtonPress = currentMillis;
      clearLcd(lcd);
      Serial.println("CLEARING AISLE 6");

      // Change menu screen
      if (menuValue < maxMenuValue){
        menuValue += 1;
      }
      else{
        menuValue = 0;
      }
    }
  }

}
