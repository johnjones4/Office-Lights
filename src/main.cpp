#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN     5
#define NUM_LEDS    30
#define BRIGHTNESS  64
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define UPDATE_DELAY_MIN 10
#define UPDATE_DELAY_MAX 1000

#define COLOR_INC_DELAY 100
#define COLOR_INC_MIN 0.001
#define COLOR_INC_MAX 0.05

#define INPUT_DEBOUNCE 500
#define COLOR_POT A0
#define SPEED_POT A1
#define MODE_SWITCH 2
#define COLOR_SWITCH 3
#define POT_MAX 1023
#define TRAIL_LENGTH 10

enum Mode {
  ChaseUp = 0,
  ChaseDown = 1,
  Bounce = 2,
  Solid = 3
};
#define MODES 4

unsigned long lastUpdate = 0;
unsigned long lastButtonDown = 0;
unsigned long lastColorInc = 0;
int currentLight = 0;
enum Mode currentMode = ChaseUp;
int delta = 1;
float colorTick = 0;

void setup() {
  Serial.begin(9600);
  pinMode(MODE_SWITCH, INPUT_PULLUP);
  pinMode(COLOR_SWITCH, INPUT_PULLUP);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  Serial.println("Ready");
}

// Helper function to convert hue to RGB
float hue_to_rgb(float p, float q, float t) {
    if (t < 0.0f) t += 1.0f;
    if (t > 1.0f) t -= 1.0f;
    if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
    if (t < 1.0f / 2.0f) return q;
    if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
    return p;
}

// Function to convert HSL to RGB
void hsl_to_rgb(float h, float s, float l, uint8_t *r, uint8_t *g, uint8_t *b) {
    float q, p;

    if (s == 0.0f) {
        // Achromatic (grey)
        *r = *g = *b = (uint8_t)(l * 255.0f);
    } else {
        if (l < 0.5f) {
            q = l * (1.0f + s);
        } else {
            q = l + s - l * s;
        }
        p = 2.0f * l - q;

        float red = hue_to_rgb(p, q, h + 1.0f / 3.0f);
        float green = hue_to_rgb(p, q, h);
        float blue = hue_to_rgb(p, q, h - 1.0f / 3.0f);

        // Convert from float [0, 1] to uint8_t [0, 255]
        *r = (uint8_t)(red * 255.0f);
        *g = (uint8_t)(green * 255.0f);
        *b = (uint8_t)(blue * 255.0f);
    }
}

float input_to_float(int pin, float min, float max) {
  int reading = analogRead(pin);
  float pcnt = (float)reading / POT_MAX;
  return (pcnt * (max - min)) + min;  
}

void update_led(int index, CRGB color) {
  int realIndex = index;
  if (index < 0) {
    realIndex = NUM_LEDS + index;
  } else if (index >= NUM_LEDS) {
    realIndex = index - NUM_LEDS;
  }
  leds[realIndex] = color;
}

void loop() {
  float colorValue;

  if (digitalRead(COLOR_SWITCH) == LOW) {
    colorValue = input_to_float(COLOR_POT, 0, 1);
  } else if (millis() - lastColorInc > COLOR_INC_DELAY) {
    lastColorInc = millis();
    colorTick += (1 - input_to_float(COLOR_POT, 0, 1)) * (COLOR_INC_MAX - COLOR_INC_MIN) + COLOR_INC_MIN;
    colorValue = sin(colorTick)/2 + 0.5;
  }

  if (digitalRead(MODE_SWITCH) == LOW && millis() - lastButtonDown > INPUT_DEBOUNCE) {
    lastButtonDown = millis();
    currentMode = (Mode)((currentMode + 1) % MODES);
  }
  
  unsigned long delay = (unsigned long)input_to_float(SPEED_POT, UPDATE_DELAY_MIN, UPDATE_DELAY_MAX);
  if (millis() - lastUpdate > delay) {
    lastUpdate = millis();
    CRGB nextColor[TRAIL_LENGTH];
    for (int i = 0; i < TRAIL_LENGTH; i++) {
      float v = square(((float)i / (float)TRAIL_LENGTH)-1);
      hsl_to_rgb(colorValue, 1, v *0.5, &nextColor[i].r, &nextColor[i].g, &nextColor[i].b);
    }
    switch (currentMode)
    {
    case ChaseUp:
      delta = 1;
      break;
    case ChaseDown:
      delta = -1;
      break;
    case Bounce:
      if (currentLight == 0 && delta == -1) {
        delta = 1;
      } else if (currentLight == NUM_LEDS -1 && delta == 1) {
        delta = -1;
      }
      break;
    case Solid:
      delta = 0;
      currentLight = 0;
      for (int i = 0; i < NUM_LEDS; i++) {
        update_led(i, nextColor[0]);
      }
    }
    if (delta != 0) {
      for (int i = 0; i < TRAIL_LENGTH; i++) {
        update_led(currentLight - (i * delta), CRGB::Black);
      }
      currentLight += delta;
      if (currentLight < 0) {
        currentLight = NUM_LEDS - 1;
      } else if (currentLight >= NUM_LEDS) {
        currentLight = 0;
      }
      for (int i = 0; i < TRAIL_LENGTH; i++) {
        update_led(currentLight - (i * delta), nextColor[i]);
      }
    }
    FastLED.show();
  }
}
