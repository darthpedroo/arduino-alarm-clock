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
// 3: Show all alarms

unsigned long lastButtonPress = 0; // Time of the last button press
unsigned long lastScrollJoystick = 0; 
unsigned long debounceDelay = 1500;  // Debounce delay in milliseconds (adjust as needed)
unsigned long debounceDelayScroll = 150;  // Debounce delay in milliseconds (adjust as needed)

int menuValue = 0;
int maxMenuValue = 3;

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


//ALARMS ARRAY

class Alarm {
public:
    int hour;
    int minute;
    int second;
    bool isActive;

    Alarm() {
        hour = 0;
        minute = 0;
        second = 0;
        isActive = false;
    }

    void setAlarm(int h, int m, int s) {
        hour = h;
        minute = m;
        second = s;
        isActive = true;
    }

    void disable() {
        isActive = false;
    }

    bool checkAlarm(int currentHour, int currentMinute, int currentSecond) {
        return isActive && hour == currentHour && minute == currentMinute && second == currentSecond;
    }
};

const int MAX_ALARMS = 5;
int alarmCounter = 0;
Alarm alarms[MAX_ALARMS];

bool isHandlingAlarm = false;
bool madeAlarmChanges = false;

#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  466
#define REST      0

int melody[] = {
  NOTE_E4, 4, NOTE_E4, 4, NOTE_F4, 4, NOTE_E4, 4,   // First measure
  NOTE_E4, 4, NOTE_G4, 4, NOTE_E4, 4, REST, 4,      // Second measure
  NOTE_A4, 8, NOTE_B4, 8, NOTE_A4, 4, REST, 4,      // Third measure
  NOTE_G4, 4, NOTE_E4, 4, REST, 4, NOTE_C4, 4       // Fourth measure
};


void turnBuzzerOn() {
  digitalWrite(buzzerPin, HIGH); // Set the buzzer pin to HIGH
}

void turnBuzzerOff() {
  digitalWrite(buzzerPin, LOW); // Set the buzzer pin to LOW
}


void handleAlarm(RtcDateTime now) {
  for (int i = 0; i < MAX_ALARMS; i++) {
    Alarm currentAlarm = alarms[i];

    // Check if the current time matches the alarm time
    if (now.Hour() == currentAlarm.hour && now.Minute() == currentAlarm.minute && now.Second() == currentAlarm.second) {
      Serial.println("ALARM CLOCK TRIGGERED!");
      printAlarmMessageLcd(lcd);

      unsigned long buzzerStartMillis = millis();  // Start the timer
      unsigned long melodyStartMillis = millis(); // Start the melody timer

      // Define the melody and note durations
      int melody[] = {
        NOTE_E4, 4, NOTE_E4, 4, NOTE_F4, 4, NOTE_E4, 4,   // First measure
        NOTE_E4, 4, NOTE_G4, 4, NOTE_E4, 4, REST, 4,      // Second measure
        NOTE_A4, 8, NOTE_B4, 8, NOTE_A4, 4, REST, 4,      // Third measure
        NOTE_G4, 4, NOTE_E4, 4, REST, 4, NOTE_C4, 4       // Fourth measure
      };
      int melodyLength = sizeof(melody) / sizeof(melody[0]) / 2;  // Number of notes in the melody
      int noteIndex = 0;
      unsigned long noteStartMillis = millis();  // Track time for each note

      // Play the melody and allow joystick interaction
      while (millis() - buzzerStartMillis < 60000) {  // Run alarm for 60 seconds
        // If it's time for the next note
        if (millis() - noteStartMillis >= (melody[noteIndex * 2 + 1] * 250)) {
          int note = melody[noteIndex * 2];   // Note frequency
          int duration = melody[noteIndex * 2 + 1];  // Duration of the note

          // Play the current note
          tone(9, note, duration * 250); // Note duration in milliseconds
          
          // Move to the next note
          noteIndex++;
          if (noteIndex >= melodyLength) {
            noteIndex = 0; // Restart the melody if it finished
          }

          // Reset the note timer
          noteStartMillis = millis();
        }

        // Check joystick click for snooze or stop
        if (isJoystickClicked(buttonPin)) {
          // Stop the alarm when the joystick is clicked
          noTone(9);  // Stop the tone immediately
          int alarmMinute = currentAlarm.minute;
          int alarmHour = currentAlarm.hour;

          unsigned long currentMillis = millis();
          lastButtonPress = currentMillis;
          alarmMinute += currentSnooze;

          if (alarmMinute >= 60) {
            alarmMinute -= 60;
            alarmHour += 1;
          }

          if (alarmHour >= 24) {
            alarmHour = 0;
          }

          // Reset the menuValue to 0 as requested
          menuValue = 0;

          alarms[i].setAlarm(alarmHour, alarmMinute, 0); // Set snooze alarm
          break;  // Exit the alarm loop when snooze is pressed
        }
      }

      // Ensure the buzzer is off after the alarm duration
      noTone(9);
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
    printTimeLcd(now, lcd, currentPhrase, 0,0);
  }
}


void handleAlarmSetup() {
    unsigned long currentMillis = millis();
    
    if (alarmCounter >= MAX_ALARMS){
      menuValue ++;
    }
    if (currentMillis - lastScrollJoystick >= debounceDelayScroll) {
        lastScrollJoystick = currentMillis;
        isHandlingAlarm = true;

        // Adjust hours
        if (isPinHigh(xPin)) {
          madeAlarmChanges = true;
            if (alarms[alarmCounter].hour >= 23) {
                alarms[alarmCounter].hour = 0;
            } else {
                alarms[alarmCounter].hour++;
            }
        } else if (isPinLow(xPin)) {
          madeAlarmChanges = true;
            if (alarms[alarmCounter].hour <= 0) {
                alarms[alarmCounter].hour = 23;
            } else {
                alarms[alarmCounter].hour--;
            }
        }        
        if (isPinLow(yPin)) {
          madeAlarmChanges = true;
            if (alarms[alarmCounter].minute >= 59) {
                alarms[alarmCounter].minute = 0;
            } else {
                alarms[alarmCounter].minute++;
            }
        } else if (isPinHigh(yPin)) {
          madeAlarmChanges = true;
            if (alarms[alarmCounter].minute <= 0) {
                alarms[alarmCounter].minute = 59;
            } else {
                alarms[alarmCounter].minute--;
            }
        }
    }

    
}

void showAllAlarms(LiquidCrystal_I2C &lcd) {
    static unsigned long lastDisplayMillis = 0;  // To track the time of the last alarm display
    unsigned long currentMillis = millis();
    static int alarmIndex = 0;  // To keep track of which alarm to display

    // If enough time has passed since the last alarm was shown, update the display
    if (currentMillis - lastDisplayMillis >= 2000) {  // 2000 ms delay between alarm displays
        lastDisplayMillis = currentMillis;  // Update the last display time

        // Clear the screen and display the current alarm
        lcd.clear();
        Alarm currentAlarm = alarms[alarmIndex];
        lcd.setCursor(0, 0);
        lcd.print("Alarm ");
        lcd.print(alarmIndex + 1);  
        lcd.print(": ");
        lcd.print(currentAlarm.hour);
        lcd.print(":");
        if (currentAlarm.minute < 10) {
            lcd.print("0");
        }
        lcd.print(currentAlarm.minute);

        alarmIndex++;
        if (alarmIndex >= MAX_ALARMS) {
            alarmIndex = 0;  
        }
    }

    if (isJoystickClicked(buttonPin)) {
        if (currentMillis - lastButtonPress >= debounceDelay) {
            lastButtonPress = currentMillis;  
            menuValue++;  
            if (menuValue > maxMenuValue) {
                menuValue = 0;  
            }
        }
    }
}



void menu(int menuValue, RtcDateTime now) {
  
  if (menuValue == 0){
    handleTimePrinting(now);
    handleAlarm(now);
  }
  else if (menuValue == 1){

    int alarmHour = alarms[alarmCounter].hour;
    int alarmMinute = alarms[alarmCounter].minute; 
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
    else if (menuValue == 3) {
    showAllAlarms(lcd);
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
      if (isHandlingAlarm && madeAlarmChanges) {
        alarmCounter ++;
      }
      isHandlingAlarm = false;
      madeAlarmChanges = false;

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
