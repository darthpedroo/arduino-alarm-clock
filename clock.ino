#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Pin configuration for DS1302 RTC module
const int RTC_CLK = 7;  // Clock Pin (SCLK)
const int RTC_DAT = 8;  // Data Pin (I/O)
const int RTC_RST = 6;  // Chip Enable Pin (CE)

// Create an instance of the DS1302 RTC
ThreeWire myWire(RTC_DAT, RTC_CLK, RTC_RST);
RtcDS1302<ThreeWire> Rtc(myWire);

int lastSecond = -1;  // Variable to store the last second checked

// Alarm variables:
int hour = 16;
int minute = 53;
int second = 0;

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

unsigned long lastButtonPress = 0; // Time of the last button press
unsigned long lastScrollJoystick = 0; 
unsigned long debounceDelay = 300;  // Debounce delay in milliseconds (adjust as needed)
unsigned long debounceDelayScroll = 150;  // Debounce delay in milliseconds (adjust as needed)

int menuValue = 0;
int maxMenuValue = 1;

void turnBuzzerOn() {
  digitalWrite(buzzerPin, HIGH); // Set the buzzer pin to HIGH
}

void turnBuzzerOff() {
  digitalWrite(buzzerPin, LOW); // Set the buzzer pin to LOW
}

void printAlarmMessageLcd(LiquidCrystal_I2C lcd) {
  lcd.setCursor(0, 0);
  lcd.print("WAKE UP");
  lcd.setCursor(0, 1);
  lcd.print("WAKE UP");
}

void handleAlarm(int hour, int minute, int second, RtcDateTime now) {
  if (now.Hour() == hour && now.Minute() == minute && now.Second() == second) {
    Serial.println("ALARM CLOCK TRIGGERED!");
    printAlarmMessageLcd(lcd);
    
    for (int i = 0; i < 5; i++) {
      turnBuzzerOn();
      delay(1000);
      turnBuzzerOff();
      delay(1000);
    }
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
    printTimeLcd(now, lcd);
  }
}

void printTime(RtcDateTime now) {
  Serial.print("Date: ");
  Serial.print(now.Day());
  Serial.print("/");
  Serial.print(now.Month());
  Serial.print("/");
  Serial.print(now.Year());
  Serial.print(" Time: ");
  Serial.print(now.Hour());
  Serial.print(":");
  Serial.print(now.Minute());
  Serial.print(":");
  Serial.println(now.Second());
}

void printTimeLcd(RtcDateTime now, LiquidCrystal_I2C lcd) {
  // Format the time and pad it to 16 characters
  String time = String(now.Hour()) + ":" + 
                String(now.Minute()) + ":" + 
                String(now.Second());
  while (time.length() < 16) {
    time += " ";
  }

  // Print the time and the second line
  lcd.setCursor(0, 0);
  lcd.print(time);
  lcd.setCursor(0, 1);
  String phrase = "stay hard";

  while (phrase.length() < 16) {
    phrase += " ";
  }
  lcd.print(phrase);
}

void printAlarmSetupLcd(LiquidCrystal_I2C lcd, int hour, int minute) {
  lcd.setCursor(0,0);
  String phrase = "SET ALARM";
  while (phrase.length() < 16) {
    phrase += " ";
  }
  lcd.print(phrase);
  lcd.setCursor(0,1);
  String time = String(hour) + " : " + String(minute);
  
  while (time.length()<16) {
    time += " ";

  }
  lcd.print(time);
}



void handleAlarmSetup(){
    
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastScrollJoystick >= debounceDelayScroll) {

      lastScrollJoystick = currentMillis;
  
      if (isPinHigh(xPin) ) {
        if (hour >= 23) {
          hour = 0;}
        else {
          hour++;
        }
      }
      else if (isPinLow(xPin)) {
        if (hour <= 0){
          hour = 23;
        }
        else{
          hour--;
        }
      }
      else if (isPinLow(yPin)) {
        if ( minute >= 59) {
          minute = 0;
        }
        else {
          minute ++;
        }
      }
      else if (isPinHigh(yPin)) {
        if ( minute <= 0) {
          minute = 59;
        }
        else {
          minute --;
        }

      }

    }
}

void menu(int menuValue, RtcDateTime now) {
  
  if (menuValue == 0){
    handleTimePrinting(now);
    handleAlarm(hour, minute, second, now);
  }
  else if (menuValue == 1){
    printAlarmSetupLcd(lcd, hour,minute);
    handleAlarmSetup();
  }

}

void setup() {
  Serial.begin(9600);  // Start serial communication
  Rtc.Begin();         // Initialize the RTC

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

      // Change menu screen
      if (menuValue < maxMenuValue){
        menuValue += 1;
      }
      else{
        menuValue = 0;
      }
    }
  }

  
  
  //printJoystick(xPin, yPin, buttonPin);
}
