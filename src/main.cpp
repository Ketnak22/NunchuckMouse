/*
  NunchuckMouse: Wii Nunchuck as a USB mouse on the RP2040_Wii board
  (see hardware/). Controls, LED and wiring are described in README.md.

  The Nunchuck runs on 3.3 V only. Anything higher will damage it.
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

#define DEADZONE 8 // raw stick units (0-255)

// If the stick reads further than this from 128 when plugged in, someone is
// holding it, so keep 128 instead of calibrating to a tilted stick.
#define MAX_CENTER_OFFSET 25

// WS2812 status LED on the RP2040-Zero
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
  // Release both buttons, or a click held while unplugging stays pressed on the PC.
  Mouse.release(MOUSE_LEFT);
  Mouse.release(MOUSE_RIGHT);
  // A replugged Nunchuck has to be initialized again before it sends data.
  connected = false;
  showStatus(false);
}

void setup() {
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
      delay(100); // Retry in a moment.
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

    // My Nunchuck rests at X = 124, Y = 128 instead of 128, 128, so take the
    // center from the first reading after plugging in.
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
    // Still needed with DET. When unplugging, I2C fails before DET goes low.
    disconnect();
  }
  delay(1);
}
