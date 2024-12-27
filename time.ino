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

