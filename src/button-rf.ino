const int buttonPin = 2;     // the number of the pushbutton pin
int buttonState = 0;         // variable for reading the pushbutton status
#include <RCSwitch.h>
RCSwitch mySwitch = RCSwitch();

void setup() {
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);
  // initialize the serial communication:
  Serial.begin(9600);
  // Transmitter is connected to Arduino Pin #10
  mySwitch.enableTransmit(10);
  mySwitch.setPulseLength(1);

}
void loop() {
  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == LOW) {
    Serial.println("Button not pressed");
  } else {
    // TODO: 
    // 1. send signal to electric plug. 
    // 2. send signal to basket to activate logic
    // 3. wait 15 seconds for game loop
    Serial.println("pressed");
    // 111101000011011001010100 turn electric plug on
    // 111101000011011001010010 turn electric plug off
    mySwitch.send("111101000011011001010100");
    delay(1000);  
    mySwitch.send("111101000011011001010010");
    delay(1000);
    Serial.println("donene");
  }

  // delay a little bit to avoid bouncing
  delay(100);
}








