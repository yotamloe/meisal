#include <FastLED.h>

#define NUM_LEDS 256  // 32*8
#define DATA_PIN 8  // Change to the pin you have connected
CRGB leds[NUM_LEDS];

const byte width = 8;
const byte height = 32;

// Font 5x3 (width x height) for numbers to display horizontally. 
byte numbers[10][5] = {
  {0b111, 0b101, 0b101, 0b101, 0b111},  // 0
  {0b010, 0b010, 0b010, 0b010, 0b010},  // 1
  {0b111, 0b100, 0b111, 0b001, 0b111},  // 2
  {0b111, 0b100, 0b111, 0b100, 0b111},  // 3
  {0b101, 0b101, 0b111, 0b001, 0b001},  // 4
  {0b111, 0b001, 0b111, 0b100, 0b111},  // 5
  {0b111, 0b001, 0b111, 0b101, 0b111},  // 6
  {0b111, 0b100, 0b100, 0b100, 0b100},  // 7
  {0b111, 0b101, 0b111, 0b101, 0b111},  // 8
  {0b111, 0b101, 0b111, 0b100, 0b100}   // 9
};

int XY(byte x, byte y) {
  if (y & 0x01) {
    return (y * width) + x;
  } else {
    return (y * width) + (width - 1 - x);
  }
}

void displayNumber(int num, int ofsetx, int ofsety) {
  for (byte x = 0; x < 5; x++) {
    for (byte y = 0; y < 3; y++) {
      if (numbers[num][x] & (0b001 << (2-y))) {  // 180-degree rotation
        // Centering the 5x3 number in the 8x8 matrix.
        leds[XY(x + ofsetx, y + ofsety)] = CRGB::Red; 
      }
    }
  }
  FastLED.show();
}

void clearMatrix() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(35);
}

void loop() {
  for (int i = 10; i >= 0; i--) {
    clearMatrix();
    if (i == 10) {
      displayNumber(1, 1, 15);
      displayNumber(0, 1, 12);
    } else {
      displayNumber(i, 1, 14);
    }
    delay(1500);  // wait for a second
  }
  clearMatrix();
  delay(1000);
}
