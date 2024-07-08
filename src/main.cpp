#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN     5
#define NUM_LEDS    20
#define BRIGHTNESS  64
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define UPDATE_DELAY 50

#define COLOR_POT A0
#define SPEED_POT A1
#define MODE_SWITCH 2
#define POT_MAX 1023

unsigned long lastUpdate = 0;
int currentLight = 0;

void setup() {
  Serial.begin(9600);
  pinMode(MODE_SWITCH, INPUT_PULLUP);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  Serial.println("Ready");
}

float hue_to_rgb(float p, float q, float t) {
  if (t < 0.0f) t += 1.0f;
  if (t > 1.0f) t -= 1.0f;
  if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
  if (t < 1.0f / 2.0f) return q;
  if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
  return p;
}

// Function to convert hue to RGB
void hue_to_rgb(float hue, uint8_t *r, uint8_t *g, uint8_t *b) {
  float q = 1.0f;
  float p = 0.0f;

  float red = hue_to_rgb(p, q, hue + 1.0f / 3.0f);
  float green = hue_to_rgb(p, q, hue);
  float blue = hue_to_rgb(p, q, hue - 1.0f / 3.0f);

  // Convert from float [0, 1] to int [0, 255]
  *r = (uint8_t)(red * 255.0f);
  *g = (uint8_t)(green * 255.0f);
  *b = (uint8_t)(blue * 255.0f);
}

float input_to_float(int pin) {
  return (float)(POT_MAX - analogRead(pin)) / POT_MAX;
}

void loop() {
  if (millis() - lastUpdate > UPDATE_DELAY) {
    float colorValue = input_to_float(COLOR_POT);
    CRGB nextColor;
    hue_to_rgb(colorValue, &nextColor.r, &nextColor.g, &nextColor.b);
    leds[currentLight] = nextColor;
    lastUpdate = millis();
    currentLight = (currentLight + 1) % NUM_LEDS;
    FastLED.show();
  }
}
