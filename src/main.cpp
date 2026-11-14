/*
  Wii Nunchuck as a USB mouse, running on the RP2040-Mini of the RP2040_Wii
  board (see hardware/).

  Controls:
    - joystick: move the cursor
    - Z: left click
    - C: right click

  Status LED (WS2812 on GP16): red = no Nunchuck, green = working.

  Nunchuck socket on the board:
    - SDA: GP0
    - SCL: GP1
    - DET: GP11, HIGH while a Nunchuck is plugged in
    - +: 3.3 V (anything over 3.3 V will fry the Nunchuck)
    - -: GND
*/

#include <Arduino.h>
#include <Mouse.h>
#include <NintendoExtensionCtrl.h>
#include <Adafruit_NeoPixel.h>

Nunchuk nchuk;
bool connected = false;
bool calibrated = false;
int failedReads = 0;
int centerX = 128;
int centerY = 128;

#define SENSITIVITY 10

// Raw joystick units (0-255) around the center that count as no movement
#define DEADZONE 8

// If the stick reads further than this from 128 when plugged in, someone is
// holding it, so keep 128 instead of calibrating to a tilted stick.
#define MAX_CENTER_OFFSET 25

// WS2812 status LED on the RP2040-Mini
#define LED_PIN 16
#define LED_BRIGHTNESS 20 // out of 255, full brightness is blinding

// A freshly connected Nunchuck often fails its first reads, so one failure
// doesn't mean it was unplugged.
#define MAX_FAILED_READS 10

#define SDA_PIN 0
#define SCL_PIN 1

// Tied to 3.3 V inside the Nunchuck, so it reads HIGH only when one is plugged in
#define DET_PIN 11

Adafruit_NeoPixel led(1, LED_PIN, NEO_GRB + NEO_KHZ800);

void showStatus(bool ok) {
  if (ok) {
    led.setPixelColor(0, led.Color(0, LED_BRIGHTNESS, 0));
  } else {
    led.setPixelColor(0, led.Color(LED_BRIGHTNESS, 0, 0));
  }
  led.show();
}

void disconnect() {
  // Otherwise a click held while unplugging stays pressed on the PC.
  Mouse.release(MOUSE_LEFT);
  Mouse.release(MOUSE_RIGHT);
  // A replugged Nunchuck has to be initialized again before it sends data.
  connected = false;
  showStatus(false);
}

void setup() {
  // Mouse.begin() re-enumerates USB, so it goes first to make the mouse show up right away.
  Mouse.begin();

  led.begin();
  showStatus(false);

  pinMode(DET_PIN, INPUT_PULLDOWN);

  Wire.setSDA(SDA_PIN);
  Wire.setSCL(SCL_PIN);
  Wire.begin();

  nchuk.begin();
}

void loop() {
  if (digitalRead(DET_PIN) == LOW) {
    if (connected) {
      disconnect();
    }
    return;
  }

  if (!connected) {
    connected = nchuk.connect();
    if (!connected) {
      delay(100); // No need to poll an empty socket at full speed
      return;
    }
    failedReads = 0;
    calibrated = false;
    showStatus(true);
  }

  if (nchuk.update()) {
    failedReads = 0;

    int joyX = nchuk.joyX();
    int joyY = nchuk.joyY();

    // Every stick rests a bit off 128, so take the center from the first
    // reading after plugging in.
    if (!calibrated) {
      centerX = abs(joyX - 128) <= MAX_CENTER_OFFSET ? joyX : 128;
      centerY = abs(joyY - 128) <= MAX_CENTER_OFFSET ? joyY : 128;
      calibrated = true;
    }

    int dx = joyX - centerX;
    int dy = joyY - centerY;
    if (abs(dx) <= DEADZONE) dx = 0;
    if (abs(dy) <= DEADZONE) dy = 0;

    int mouseX = dx * SENSITIVITY / 128;
    int mouseY = -dy * SENSITIVITY / 128; // Stick up is +Y, screen up is -Y

    if (mouseX != 0 || mouseY != 0) {
      Mouse.move(mouseX, mouseY);
    }

    if (nchuk.buttonZ()) {
      Mouse.press(MOUSE_LEFT);
    } else {
      Mouse.release(MOUSE_LEFT);
    }

    if (nchuk.buttonC()) {
      Mouse.press(MOUSE_RIGHT);
    } else {
      Mouse.release(MOUSE_RIGHT);
    }
  } else if (++failedReads >= MAX_FAILED_READS) {
    // Still needed with DET: when unplugging, I2C fails before DET goes low.
    disconnect();
  }
  delay(1); // Brief delay to prevent overwhelming Nunchuck with readings

}
