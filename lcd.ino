void clearLcd(LiquidCrystal_I2C lcd) {
  
  String emptyPadding = "";

  while (emptyPadding.length() < 16){
    emptyPadding += " ";
  }
  
  lcd.setCursor(0, 0);
  lcd.print(emptyPadding);
  lcd.setCursor(0, 1);
  lcd.print(emptyPadding);

}
