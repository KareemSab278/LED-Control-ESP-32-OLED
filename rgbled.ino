/*
  The way this works is you send commands to it like:
    "NUMBER COLOR" over serial.
    so to set led strip 2 to white youll send: "2 white".

  if no number specified then just set all to the color

  this is so it can work with express much like vend_mini - cross compatable.

  YOU MUST follow the setup of DIN_PINS in order (supports up to 5 pins)
     DIAGRAM
  *___________*
  | pin ->  # |
  |___________|
  | 23  ->  1 |
  |___________|
  | 22  ->  2 |
  |___________|
  | 21  ->  3 |
  |___________|
  | 19  ->  4 |
  |___________|
  | 18  ->  5 |
  *___________*

*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FastLED.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define NUM_STRIPS 5
#define NUM_LEDS 148
#define BRIGHTNESS 30

const uint8_t DIN_PINS[NUM_STRIPS] = {
  23, 22, 21, 19, 18
};

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET);

// Each strip has its own LED array and current colour.
CRGB leds[NUM_STRIPS][NUM_LEDS];
CRGB currentColours[NUM_STRIPS];

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

const int colourCount =
  sizeof(colours) / sizeof(colours[0]);

String command = "";

void updateOLED(String text) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(0, 25);
  display.println(text);
  display.display();
}


void fadeToColour(int strip, CRGB targetColour) {
  int firstStrip = (strip == -1) ? 0 : strip;
  int lastStrip = (strip == -1) ? NUM_STRIPS - 1 : strip;

  CRGB startColours[NUM_STRIPS];

  for (int s = firstStrip; s <= lastStrip; s++) {
    startColours[s] = currentColours[s];
  }

  const int steps = 100;
  const int fadeTime = 1000;
  const int delayTime = fadeTime / steps;

  for (int i = 1; i <= steps; i++) {
    uint8_t amount = map(i, 0, steps, 0, 255);

    for (int s = firstStrip; s <= lastStrip; s++) {
      CRGB blended = blend(
        startColours[s],
        targetColour,
        amount);

      fill_solid(leds[s], NUM_LEDS, blended);
    }

    FastLED.show();
    delay(delayTime);
  }

  for (int s = firstStrip; s <= lastStrip; s++) {
    currentColours[s] = targetColour;
  }
}



bool setColourFromCommand(String cmd) {
  cmd.trim();
  cmd.toLowerCase();

  int separator = cmd.indexOf(' ');
  int firstStrip = 0;
  int lastStrip = NUM_STRIPS - 1;
  String colourName = cmd;

  // If a strip number is supplied, target only that strip.
  if (separator != -1) {
    String stripText = cmd.substring(0, separator);
    colourName = cmd.substring(separator + 1);
    colourName.trim();

    // Ensure the strip number is numeric.
    for (int i = 0; i < stripText.length(); i++) {
      if (!isDigit(stripText[i])) {
        return false;
      }
    }

    int stripNumber = stripText.toInt();

    if (stripNumber < 1 || stripNumber > NUM_STRIPS) {
      return false;
    }

    firstStrip = stripNumber - 1;
    lastStrip = stripNumber - 1;
  }

  for (int i = 0; i < colourCount; i++) {
    if (colourName == colours[i].name) {
      String oledText;

      if (firstStrip == lastStrip) {
        oledText = "LED " + String(firstStrip + 1)
                   + " " + colourName;
      } else {
        oledText = "ALL " + colourName;
      }

      updateOLED(oledText);

      if (firstStrip == 0 && lastStrip == NUM_STRIPS - 1) {
        fadeToColour(-1, colours[i].value);
      } else {
        fadeToColour(firstStrip, colours[i].value);
      }

      Serial.print("Set ");
      if (firstStrip == lastStrip) {
        Serial.print("LED ");
        Serial.print(firstStrip + 1);
      } else {
        Serial.print("ALL LEDs");
      }

      Serial.print(" to ");
      Serial.println(colourName);

      return true;
    }
  }

  return false;
}


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

  updateOLED("LED READY");

  FastLED.addLeds<WS2812B, 23, GRB>(leds[0], NUM_LEDS);
  FastLED.addLeds<WS2812B, 22, GRB>(leds[1], NUM_LEDS);
  FastLED.addLeds<WS2812B, 21, GRB>(leds[2], NUM_LEDS);
  FastLED.addLeds<WS2812B, 19, GRB>(leds[3], NUM_LEDS);
  FastLED.addLeds<WS2812B, 18, GRB>(leds[4], NUM_LEDS);

  FastLED.setBrightness(BRIGHTNESS);

  for (int i = 0; i < NUM_STRIPS; i++) {
    currentColours[i] = CRGB::Black;
    fill_solid(leds[i], NUM_LEDS, CRGB::Black);
  }

  FastLED.show();

  Serial.println("LED READY");
  Serial.println("Commands: <led_number> <colour>");
  Serial.println("Examples: 1 red | 2 blue | 5 green");
  Serial.println("Colours: red green blue white yellow none");
  Serial.println("Use ident? to check connection");
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') {
      command.trim();
      command.toLowerCase();

      if (command == "ident?") {
        Serial.println("LEDACK");
      } else if (!setColourFromCommand(command)) {
        Serial.print("Unknown command: ");
        Serial.println(command);
      }

      command = "";
    } else {
      command += c;
    }
  }
}
