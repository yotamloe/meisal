#include <FastLED.h>

// LED strip setup
#define LED_STRIP_PIN 5
#define NUM_STRIP_LEDS 100
CRGB stripLeds[NUM_STRIP_LEDS];
uint8_t colorIndex[NUM_STRIP_LEDS];

// LED matrix setup
#define LED_MATRIX_PIN 4
#define NUM_MATRIX_LEDS 256
#define MATRIX_WIDTH 32
CRGB matrixLeds[NUM_MATRIX_LEDS];

// LED effects setup
TaskHandle_t LedEffectTaskHandle = NULL;
enum class LedEffect {
  Default,
  GAME_PLAY,
  GAME_WON,
  GAME_LOST
};

volatile LedEffect currentEffect = LedEffect::Default;

DEFINE_GRADIENT_PALETTE(darkpurple_gp) {
    0, 80, 0, 130,
    46, 60, 0, 110,
    179, 40, 0, 90,
    255, 30, 0, 80
};

CRGBPalette16 darkpurple = darkpurple_gp;

DEFINE_GRADIENT_PALETTE(orange_gp) {
    0,   255, 165, 0,   // Orange
    66,  255, 140, 0,   // Darker Orange
    179, 255, 90,  0,   // Even Darker Orange
    255, 200, 50,  0    // Dark Orange
};

CRGBPalette16 orange = orange_gp;

// Photoelectric sensor
#define SENSOR_PIN 27

// push button
const int buttonPin = 2;     // the number of the pushbutton pin
int buttonState = 0;         // variable for reading the pushbutton status

// mp3
#define CMD_PLAY_W_INDEX 0X03
#define GAME_SOUND 1 // 099.mp3 is the game music
#define BUZZER_SOUND 2 // 066.mp3 is the buzzer sound 


HardwareSerial mySerial(2); // Use second hardware serial. RX, TX

unsigned long startTime;
bool gameStarted = false;
int countDown = 10;


unsigned long previousMillis = 0; 

void ledEffectTask(void * parameter) {
  while (true) {
    unsigned long currentMillis = millis();
    
    switch (currentEffect) {
      case LedEffect::GAME_PLAY:
          if(currentMillis - previousMillis > 10) {              
              // Create a sin wave to change the brightness of the strip
              uint8_t sinBeat = beatsin8(95, 40, 130, 10, 0);
              
              // Color each pixel from the palette using the index from colorIndex[]
              for (int i = 0; i < NUM_STRIP_LEDS; i++) {
                  stripLeds[i] = ColorFromPalette(orange, colorIndex[i], sinBeat); 
              }
              
              // Increment colorIndex values every 5 milliseconds
              EVERY_N_MILLISECONDS(5){
                  for (int i = 0; i < NUM_STRIP_LEDS; i++) {
                      colorIndex[i]++;
                  }
              }
              
              FastLED.show();
              previousMillis = currentMillis;
          }
          break;

      case LedEffect::GAME_WON:
          if(currentMillis - previousMillis > 10) {  // This interval controls the speed of the effect

              uint8_t sinBeat   = beatsin8(25, 0, NUM_STRIP_LEDS - 1, 0, 0);
              uint8_t sinBeat2  = beatsin8(10, 0, NUM_STRIP_LEDS - 1, 0, 85);
              uint8_t sinBeat3  = beatsin8(22, 0, NUM_STRIP_LEDS - 1, 0, 170);
              uint8_t sinBeat4  = beatsin8(32, 0, NUM_STRIP_LEDS - 1, 0, 170);
              uint8_t sinBeat5  = beatsin8(27, 0, NUM_STRIP_LEDS - 1, 0, 170);

              // Set green colors to different brightness levels based on the sinBeats
              stripLeds[sinBeat]   = CRGB(0, 255, 70);     // Bright Green
              stripLeds[sinBeat2]  = CRGB(0, 192, 50);     // Medium Green
              stripLeds[sinBeat3]  = CRGB(0, 128, 20);     // Dark Green
              stripLeds[sinBeat4]  = CRGB(0, 192, 50);     // Medium Green
              stripLeds[sinBeat5]  = CRGB(0, 128, 20);     // Dark Green
              // Apply blur
              for(int i = 0; i < 5; i++) {
                  blur1d(stripLeds, NUM_STRIP_LEDS, 50);
              }
              
              FastLED.show();
              fadeToBlackBy(stripLeds, NUM_STRIP_LEDS, 10);  // This helps in fading the previous colors
              previousMillis = currentMillis;
          }
          break;
      case LedEffect::GAME_LOST:
        if(currentMillis - previousMillis > 400) {
          // Your non-blocking game lost effect logic here
          static bool isRed = true;
          FastLED.showColor(isRed ? CRGB::Red : CRGB::Black);
          isRed = !isRed;
          
          previousMillis = currentMillis;
        }
        break;

      case LedEffect::Default:
      default:
          if(currentMillis - previousMillis > 10) {              
              // Create a sin wave to change the brightness of the strip
              uint8_t sinBeat = beatsin8(15, 25, 70, 0, 0);
              
              // Color each pixel from the palette using the index from colorIndex[]
              for (int i = 0; i < NUM_STRIP_LEDS; i++) {
                  stripLeds[i] = ColorFromPalette(darkpurple, colorIndex[i], sinBeat);
              }
              
              // Increment colorIndex values every 5 milliseconds
              EVERY_N_MILLISECONDS(5){
                  for (int i = 0; i < NUM_STRIP_LEDS; i++) {
                      colorIndex[i]++;
                  }
              }
              
              FastLED.show();
              previousMillis = currentMillis;
          }
          break;
    }
  }
}

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<NEOPIXEL, LED_STRIP_PIN>(stripLeds, NUM_STRIP_LEDS);
  FastLED.addLeds<NEOPIXEL, LED_MATRIX_PIN>(matrixLeds, NUM_MATRIX_LEDS);
  FastLED.setBrightness(25);
  pinMode(SENSOR_PIN, INPUT);
  mySerial.begin(9600, SERIAL_8N1, 16, 17); // Initialize Serial2 on pins 16 (RX) and 17 (TX)
  randomSeed(analogRead(0));
  // Create the LED effect task
  xTaskCreatePinnedToCore(
      ledEffectTask,   /* Task function. */
      "LedEffectTask", /* name of task. */
      10000,           /* Stack size of task (increased stack size because of the large LED arrays) */
      NULL,            /* parameter of the task */
      1,               /* priority of the task */
      &LedEffectTaskHandle, /* Task handle to keep track of created task */
      1);              /* pin task to core 1 */
  for (int i = 0; i < NUM_STRIP_LEDS; i++) {
    colorIndex[i] = random8();
  }
  delay(1000);
}

void loop() {
  if (gameStarted) {
    int elapsedSeconds = (millis() - startTime) / 1000;
    if (elapsedSeconds < 15) {
      updateCountdown(15 - elapsedSeconds);
      checkBasket();
    } else {
      gameLost();
    }
  } else {
      // read the state of the pushbutton value:
      buttonState = digitalRead(buttonPin);
      // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
      if (buttonState == HIGH) {
        startGame();
      }
    //   String command = Serial.readStringUntil('\n');
    //   if (command == "start") {
    // }
  }
}

void updateCountdown(int remaining) {
  // TODO: update your LED matrix to display the remaining time.
  displayNumberOnMatrix(remaining);
}

void checkBasket() {
// Serial.println(digitalRead(SENSOR_PIN));
  if ( digitalRead(SENSOR_PIN) == HIGH) {
    gameWon();
  }
}

void startGame() {
  Serial.println("starting game");
  gameStarted = true;
  startTime = millis();
  // TODO: better effect
  currentEffect = LedEffect::GAME_PLAY;
  Serial.println(GAME_SOUND);  
  playTrack(CMD_PLAY_W_INDEX, GAME_SOUND); 
}

void gameWon() {
  Serial.println("Game won");
  // TODO: better effect
  currentEffect = LedEffect::GAME_WON;
  int trackNumber = random(3, 7);
  playRandomSound(trackNumber);
  delay(13000);
  resetGame();
}

void gameLost() {
  Serial.println("Game lost");
  // TODO: better effect
  currentEffect = LedEffect::GAME_LOST;
  Serial.println(BUZZER_SOUND);
  playTrack(CMD_PLAY_W_INDEX, BUZZER_SOUND);
  delay(6000);
  resetGame();
}

void resetGame() {
  FastLED.clear();
  FastLED.show();
  gameStarted = false;
  currentEffect = LedEffect::Default;  // Signal to go back to Default effect
}

void playRandomSound(int trackNumber) {
  Serial.println(trackNumber);
  playTrack(CMD_PLAY_W_INDEX, trackNumber);
}

void playTrack(int8_t command, int16_t dat)
{
  delay(20);
  byte Send_buf[8] = {0}; // Buffer for Send commands. 
  Send_buf[0] = 0x7e; // Start byte
  Send_buf[1] = 0xff; // Version
  Send_buf[2] = 0x06; // Command length
  Send_buf[3] = command; // Command
  Send_buf[4] = 0x01; // 0x00 NO, 0x01 feedback
  Send_buf[5] = (byte)(dat >> 8); // datah
  Send_buf[6] = (byte)(dat); // datal
  Send_buf[7] = 0xef; // end byte
  for(uint8_t i=0; i<8; i++)//
  {
    mySerial.write(Send_buf[i]) ;// send bit to serial mp3
  }
}


void displayNumberOnMatrix(int num) {
  // Placeholder: you might need a separate function or library to display numbers on the matrix.
}
