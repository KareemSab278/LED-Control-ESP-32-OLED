#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FastLED.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET);


#define NUM_LEDS 148
#define DIN_PIN 23
#define BRIGHTNESS 30

CRGB leds[NUM_LEDS];

CRGB currentColour = CRGB::Black;


struct Colour {
  String name;
  CRGB value;
};

Colour colours[] = {
  { "red", CRGB::Red },
  { "green", CRGB::Green },
  { "blue", CRGB::Blue },
  { "white", CRGB::White },
  { "yellow", CRGB::Yellow },
  { "none", CRGB::Black }
};

const int colourCount = sizeof(colours) / sizeof(colours[0]);

void updateOLED(String colour) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(10, 30);
  display.println(colour);
  display.display();
}


void fadeToColour(CRGB targetColour) {
  CRGB startColour = currentColour;

  const int steps = 100;
  const int fadeTime = 1000;
  const int delayTime = fadeTime / steps;

  for (int i = 1; i <= steps; i++) {
    uint8_t amount = map(
      i,
      0,
      steps,
      0,
      255);

    CRGB blended = blend(
      startColour,
      targetColour,
      amount);

    fill_solid(
      leds,
      NUM_LEDS,
      blended);

    FastLED.show();
    delay(delayTime);
  }
  currentColour = targetColour;
}

String command = "";

void setup() {
  Serial.begin(115200);
  Wire.begin();

  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        SCREEN_ADDRESS)) {

    Serial.println("OLED FAILED");
    while (true)
      ;
  }

  updateOLED("none");

  FastLED.addLeds<
    WS2812B,
    DIN_PIN,
    GRB >(
    leds,
    NUM_LEDS);

  FastLED.setBrightness(
    BRIGHTNESS);

  FastLED.clear();
  FastLED.show();
  Serial.println("LED READY");
  Serial.println(
    "Commands: red green blue white yellow none");
}

bool setColourFromCommand(String command) {

  for (int i = 0; i < colourCount; i++) {

    if (command == colours[i].name) {
      // make the oled update immediately and not wait for fade
      updateOLED(command);
      fadeToColour(colours[i].value);

      return true;
    }
  }

  return false;
}


void loop() {
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') {
      command.trim();
      command.toLowerCase();

      if (command == "ident?") {
        Serial.println("LEDACK");
      }

      else if (!setColourFromCommand(command)) {
        Serial.print("Unknown: ");
        Serial.println(command);
      }

      command = "";
    } else {
      command += c;
    }
  }
}