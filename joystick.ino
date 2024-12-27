
void printJoystick(int xPin, int yPin, int buttonPin){
  
  xVal = analogRead(xPin);
  yVal = analogRead(yPin);
  buttonState = digitalRead(buttonPin);

  Serial.print(" | X: | ");
  Serial.print(xVal);
  Serial.print(" | Y: | ");
  Serial.print(yVal);
  Serial.print(" | State :| ");
  Serial.print(buttonState);
  Serial.println(" ");
  Serial.println("-------------------------------");
  //delay(400);
}

int isJoystickClicked(int buttonPin) {
  
  buttonState = digitalRead(buttonPin);
  return buttonState == LOW;
}

int isPinHigh(int Pin) {
  int value = analogRead(Pin);
  return value == 1020;
}

int isPinLow(int Pin) {
  int value = analogRead(Pin);
  return value == 0;

}
