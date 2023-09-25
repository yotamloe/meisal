#include <FastLED.h>

// LED setup
#define LED_STRIP_PIN 5
#define LED_MATRIX_PIN 4
#define NUM_STRIP_LEDS 100
#define NUM_MATRIX_LEDS 256
#define MATRIX_WIDTH 32

// Photoelectric sensor
#define SENSOR_PIN 27

// mp3
#define CMD_PLAY_W_INDEX 0X03
#define GAME_SOUND 1 // 099.mp3 is the game music
#define BUZZER_SOUND 2 // 066.mp3 is the buzzer sound 



CRGB stripLeds[NUM_STRIP_LEDS];
CRGB matrixLeds[NUM_MATRIX_LEDS];

HardwareSerial mySerial(2); // Use second hardware serial. RX, TX

unsigned long startTime;
bool gameStarted = false;
int countDown = 10;

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<NEOPIXEL, LED_STRIP_PIN>(stripLeds, NUM_STRIP_LEDS);
  FastLED.addLeds<NEOPIXEL, LED_MATRIX_PIN>(matrixLeds, NUM_MATRIX_LEDS);
  FastLED.setBrightness(25);
  pinMode(SENSOR_PIN, INPUT);
  mySerial.begin(9600, SERIAL_8N1, 17, 16); // Initialize Serial2 on pins 16 (RX) and 17 (TX)
  randomSeed(analogRead(0));
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
      String command = Serial.readStringUntil('\n');
      if (command == "start") {
      startGame();
    }
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
  FastLED.showColor(CRGB::Yellow);
  Serial.println(GAME_SOUND);  
  playTrack(CMD_PLAY_W_INDEX, GAME_SOUND); 
}

void gameWon() {
  Serial.println("Game won");
  // TODO: better effect
  FastLED.showColor(CRGB::Green);
  playRandomSound();
  resetGame();
}

void gameLost() {
  Serial.println("Game lost");
  // TODO: better effect
  FastLED.showColor(CRGB::Red);
  Serial.println(BUZZER_SOUND);
  playTrack(CMD_PLAY_W_INDEX, BUZZER_SOUND);
  resetGame();
}

void resetGame() {
  // TODO: extend
  delay(8000);
  FastLED.clear();
  FastLED.show();
  gameStarted = false;
}

void playRandomSound() {
  int trackNumber = random(3, 7);
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
