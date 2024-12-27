void clearLcd(LiquidCrystal_I2C lcd) {
  
  String emptyPadding = "";

  while (emptyPadding.length() < 40){
    emptyPadding += " ";
  }
  
  lcd.setCursor(0, 0);
  lcd.print(emptyPadding);
  lcd.setCursor(0, 1);
  lcd.print(emptyPadding);

}

void printTimeLcd(RtcDateTime now, LiquidCrystal_I2C lcd, int row, int col) {
  // Format the time and pad it to 16 characters

  String hour = String(now.Hour());
  String minute = String(now.Minute());
  String second = String(now.Second());

  if (hour.length() <2 ){
    hour = "0"+hour;
  }

  if (minute.length() <2 ){
    minute = "0"+minute;
  }

  if (second.length() <2 ){
    second = "0"+second;
  }

  String time = hour + ":" + 
                minute + ":" + 
                second;
  while (time.length() < 16) {
    time += " ";
  }

  // Print the time and the second line
  lcd.setCursor(row, col);
  lcd.print(time);
  lcd.setCursor(0, 1);
  String phrase = "stay hard";

  while (phrase.length() < 16) {
    phrase += " ";
  }
  lcd.print(phrase);
}

void printPhrase(String phrase, int row, int col){
  lcd.setCursor(row,col);
  while (phrase.length() < 16) {
    phrase += " ";
  }
  lcd.print(phrase);
}

void printAlarmSetupLcd(LiquidCrystal_I2C lcd, int hour, int minute) {
  String phrase = "SET ALARM";
  printPhrase(phrase, 0, 0);
  lcd.setCursor(0, 0);
  lcd.print(phrase);
  lcd.setCursor(0, 1);
  
  // Add leading zeros for single-digit hour and minute
  String formattedHour = (hour < 10) ? "0" + String(hour) : String(hour);
  String formattedMinute = (minute < 10) ? "0" + String(minute) : String(minute);

  String time = formattedHour + " : " + formattedMinute;

  while (time.length() < 16) {
    time += " ";
  }
  lcd.print(time);
}

void printAlarmMessageLcd(LiquidCrystal_I2C lcd) {
  printPhrase("WAKE UP", 0, 0);
  printPhrase("WAKE UP", 0, 1);
}