/*
  This project enables the use of a Nintendo Nunchuck controller as a mouse input device. 
  By using the controller's joystick, you can control mouse movements, and by using the C 
  and Z buttons, you can perform left and right clicks.

  LED Blink Codes:
  The built-in LED provides visual feedback during the initialization process. The number of 
  blinks corresponds to the following statuses:
    - 0 blinks: Wire library initialization failed
    - 1 blink: Nunchuck initialization failed
    - 2 blinks: Mouse initialization failed
    - 3 blinks: Everything initialized successfully

  Ensure that the Nunchuck is properly connected (using Raspberry Pi Pico as example):
    - SDA: physical pin 6
    - SCL: physical pin 7
    - +: 3.3V power supply (anything over 3.3V will fry your nunchuck, so be sure!)
    - -: ground of power supply
*/

#include <Mouse.h>
#include <NintendoExtensionCtrl.h>

Nunchuk nchuk;

// Declare mouse sensitivity (the lower the sensitivity, the slower the mouse)
#define SENSITIVITY 10

// Pin number of built-in LED
#define LED_PIN 25

// I2C Pins
#define SDA_PIN 4
#define SCL_PIN 5

// Blink built-in LED for half a second
void blink_led() {
  digitalWrite(LED_PIN, HIGH);   // turn the LED on
  delay(500);                       
  digitalWrite(LED_PIN, LOW);    // turn the LED off
  delay(500);
}

void setup() {
  // Setup LED Pin
  pinMode(LED_PIN, OUTPUT);

  // Initialize Wire
  Wire.setSDA(SDA_PIN);
  Wire.setSCL(SCL_PIN);
  Wire.begin();

  blink_led(); // Debug info

  // Initialize Nunchuck
  nchuk.begin();
  while (!nchuk.connect());

  blink_led(); // Debug info

  // Initialize mouse
  Mouse.begin();

  blink_led(); // Debug info
}

void loop() {
  if (nchuk.update()) {
    // Get joystick values
    int joyX = nchuk.joyX();
    int joyY = nchuk.joyY();

    // Map joystick values to mouse movement
    int mouseX = map(joyX, 0, 255, -SENSITIVITY, SENSITIVITY);
    int mouseY = -map(joyY, 0, 255, -SENSITIVITY, SENSITIVITY); // Also invert Y movement

    // Check if joystick movement is outside the dead zone
    if (abs(mouseX) > 1 || abs(mouseY) > 1) {
      Mouse.move(mouseX, mouseY);
    }

    // Press left mouse button if Z button is pressed on Nunchuck
    if (nchuk.buttonZ()) {
      Mouse.press(MOUSE_LEFT);
    } else {
      Mouse.release(MOUSE_LEFT);
    }

    // Press right mouse button if C button is pressed on Nunchuck
    if (nchuk.buttonC()) {
      Mouse.press(MOUSE_RIGHT);
    } else {
      Mouse.release(MOUSE_RIGHT);
    }
  } else {
    while (!nchuk.update()); // Wait until data is available again
  }
  delay(1); // Brief delay to prevent overwhelming Nunchuck with readings

}
