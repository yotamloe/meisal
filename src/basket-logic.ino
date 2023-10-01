#include <FastLED.h>

// LED strip setup
#define LED_STRIP_PIN 5
#define NUM_STRIP_LEDS 100
CRGB stripLeds[NUM_STRIP_LEDS];

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

// Photoelectric sensor
#define SENSOR_PIN 27

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
        // Check if it's time to update the animation (e.g., every 10 ms)
        if(currentMillis - previousMillis > 10) {
          // Your non-blocking game play effect logic here
          // For instance, the brightness fading effect:
          static int direction = 10;
          static int brightness = 0;
          
          brightness += direction;
          if (brightness >= 200) {
            direction = -10;
          } else if (brightness <= 0) {
            direction = 10;
          }
          FastLED.setBrightness(brightness);
          FastLED.showColor(CRGB::Yellow);
          
          previousMillis = currentMillis;
        }
        break;

      case LedEffect::GAME_WON:
        if(currentMillis - previousMillis > 100) {
          // Your non-blocking game won effect logic here
          for(int i = 0; i < NUM_STRIP_LEDS; i++){
            stripLeds[i] = CRGB::Black;
          }
          stripLeds[random(NUM_STRIP_LEDS)] = CRGB::Green;
          FastLED.show();
          
          previousMillis = currentMillis;
        }
        break;

      case LedEffect::GAME_LOST:
        if(currentMillis - previousMillis > 300) {
          // Your non-blocking game lost effect logic here
          static bool isRed = true;
          FastLED.showColor(isRed ? CRGB::Red : CRGB::Black);
          isRed = !isRed;
          
          previousMillis = currentMillis;
        }
        break;

      case LedEffect::Default:
      default:
        // No effect or default effect
        FastLED.showColor(CRGB::Black);
        vTaskDelay(100 / portTICK_PERIOD_MS); // Use FreeRTOS delay here.
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
