#include <Arduino.h>

#define COLOR_POT A0
#define SPEED_POT A1
#define MODE_SWITCH 2

void setup() {
  Serial.begin(9600);
  pinMode(MODE_SWITCH, INPUT_PULLUP);
  Serial.println("Ready");
}

void loop() {
  int value = analogRead(COLOR_POT);
  Serial.println(value);
}
