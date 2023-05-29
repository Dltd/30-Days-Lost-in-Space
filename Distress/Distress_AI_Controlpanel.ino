#include <RotaryEncoder.h>
#include <TM1637Display.h>
#include <U8glib.h>

// Pin definitions - I used 220 resistors for each of the leds
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3
#define ENCODER_PIN_BTN 4
#define CLK 11
#define DIO 12
#define RED_LED_PIN 5
#define YELLOW_LED_PIN 6
#define GREEN_LED_PIN 7
#define RGB_RED_PIN 8
#define RGB_GREEN_PIN 9
#define RGB_BLUE_PIN 10
#define PHOTORESISTOR_PIN A0 // one leg into GND, the other one via a resistor to VCC

// Constants
#define RESISTOR_VALUE 220000
#define SEG_D 0x40
#define BOOTING_DELAY 500
#define CHARGING_STEP 10

// Global objects
RotaryEncoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);
TM1637Display display(CLK, DIO);
U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE); // I2C / TWI  SCL to A5 and SDA to A4

// Global variables
int prevEncoderPos = 0;
int ledCycles = 0;
int currentPage = 0;
unsigned long lastPageUpdate = 0;
const int pageUpdateInterval = 50; // delay is determined by the ledIndicator function
const int totalPages = 6; // Change totalPages to 6
bool finalPageReached = false; // Add a flag to control when the final page is reached
bool aiCoreBooted = false; // Add a flag to control whne the AI core is booted

// Planet and Moon structures from progspacev2.2
struct Planet {
  float x;
  float y;
  float radius;
};

struct Moon {
  float x;
  float y;
  float radius;
  float angle;
};

// Variables from progspacev2.2
Planet planet = {64, 32, 8};
Moon moon = {0, 0, 2, 0};
float zoomFactor = 1.0;
bool animationFinished = false;

// Function prototypes for the compiler
void handleEncoder();
void ledIndicator();
void displayAIPage(int pageNumber);
void drawIcon(int x, int y);
void drawSpaceMap();

void setup() {
  // Set pin modes
  pinMode(ENCODER_PIN_BTN, INPUT_PULLUP);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);
  pinMode(PHOTORESISTOR_PIN, INPUT);

  // Set up display
  display.setBrightness(0x0f);

  // Attach the ISR function for the rotary encoder
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), handleEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), handleEncoder, CHANGE);
}

void handleEncoder() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();

  // Debounce the rotary encoder
  if (interruptTime - lastInterruptTime > 3) {
    encoder.tick();
  }

  lastInterruptTime = interruptTime;
}

void ledIndicator() {
  // Fast animated LED cycle
  for (int k = 0; k < 5; k++) {
    for (int i = 0; i < 2; i++) {
      int pins[] = {RED_LED_PIN, YELLOW_LED_PIN, GREEN_LED_PIN, RGB_RED_PIN, RGB_GREEN_PIN, RGB_BLUE_PIN, GREEN_LED_PIN, YELLOW_LED_PIN, RED_LED_PIN};
      for (int j = 0; j < 9; j++) {
        digitalWrite(pins[j], HIGH);
        delay(40 - k * 10); // Slowly decrease the delay
        digitalWrite(pins[j], LOW);
      }
    }
  }
}

// this will show the status pages, bitmaps with animated levels and meters are to be developed
void displayAIPage(int pageNumber) { 
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_profont15); // set the default font and size 
    u8g.setPrintPos(0, 20);  // set the default position for the first line of text

    switch (pageNumber) {
      case 0:
        u8g.print("QUANTUM ENTANGLEMENT");
        u8g.setFont(u8g_font_profont12);
        u8g.setPrintPos(0, 40);
        //u8g.setFont(u8g_font_profont10);       
        u8g.print("STATUS: OK");
        break;
      case 1:
        u8g.print("NEURAL NETWORK");
        u8g.setFont(u8g_font_profont12);
        u8g.setPrintPos(0, 40);
        //u8g.setFont(u8g_font_profont10);
        u8g.print("INTEGRITY: 98%");
        break;
      case 2:
        u8g.print("ENERGY SOURCE");
        u8g.setFont(u8g_font_profont12);
        u8g.setPrintPos(0, 40);
        //u8g.setFont(u8g_font_profont10);
        u8g.print("STABILITY: HI");
        break;
      case 3:
        u8g.print("TIME DILATION");
        u8g.setFont(u8g_font_profont12);
        u8g.setPrintPos(0, 40);
        //u8g.setFont(u8g_font_profont10);
        u8g.print("COEF: 1.25");
        break;
      case 4:
        u8g.print("GALACTIC POS");
        u8g.setFont(u8g_font_profont12);
        u8g.setPrintPos(0, 40);
        //u8g.setFont(u8g_font_profont10);
        u8g.print("X: 5.3, Y: 7.1");
        break;
      case 5:
        while (!digitalRead(ENCODER_PIN_BTN)) {
          drawSpaceMap();
      }
        break;
    }
  } while (u8g.nextPage());
}

void drawIcon(int x, int y) {
  const byte Icon[][8] PROGMEM = {
    0x95
  };

  u8g.drawXBMP(x, y, 16, 8, Icon[0]);
}

void drawSpaceMap() {
  u8g.firstPage();
  do {
    if (animationFinished) {
      u8g.setRot180();
      u8g.setFont(u8g_font_10x20); // Set the font size to 12
      u8g.drawStr(0, 32, "Thank You"); // Draw the "Thank You" text
      u8g.setFont(u8g_font_10x20); // Set the font size to 12

      drawIcon(35, 40); // Draw something below the text
    } else {
      // Draw stars
      for (uint8_t i = 0; i < 20; i++) {
        uint8_t x = random(0, 128);
        uint8_t y = random(0, 64);
        u8g.drawPixel(x, y);
      }

      // Draw and fill the planet
      u8g.drawCircle(planet.x, planet.y, planet.radius * zoomFactor); // Planet circle
      u8g.drawDisc(planet.x, planet.y, planet.radius * zoomFactor);   // Planet fill

      // Update the moon's angle and position in relation to the planet
      moon.angle += 0.05;
      moon.x = planet.x + (planet.radius * zoomFactor + moon.radius * zoomFactor + 2) * cos(moon.angle);
      moon.y = planet.y + (planet.radius * zoomFactor + moon.radius * zoomFactor + 2) * sin(moon.angle);

      // Draw and fill the moon
      u8g.drawCircle(moon.x, moon.y, moon.radius * zoomFactor); // Moon circle
      u8g.drawDisc(moon.x, moon.y, moon.radius * zoomFactor);   // Moon fill
    }
  } while (u8g.nextPage());
}


void loop() {
  static bool animationDone = false;
  static unsigned long lastAnimation = 0;
  static int animationStep = 0;

  if (!animationDone) {
    if (millis() - lastAnimation > 200) {
      display.clear();
      uint8_t d_segment = SEG_D;
      display.setSegments(&d_segment, 1, animationStep);
      animationStep = (animationStep + 1) % 4;
      lastAnimation = millis();
      // Display the word "push" in small caps
      const uint8_t turnSegments[] = {0x73, 0x3E, 0x6D, 0x76}; // push
      display.setSegments(turnSegments);
      delay(69);
      display.clear();
      const uint8_t turnSegments2[] = {0x71, 0x77, 0x30, 0x38}; // help
      display.setSegments(turnSegments2);
      delay(200);
      display.clear();      
    }
    // Blink RED LED and RGB LED with RED and GREEN colors fast
      digitalWrite(RED_LED_PIN, !digitalRead(RED_LED_PIN));
      delay(10);
      digitalWrite(RGB_BLUE_PIN, !digitalRead(RGB_BLUE_PIN));
      delay(10);
      digitalWrite(YELLOW_LED_PIN, !digitalRead(YELLOW_LED_PIN));
      //digitalWrite(RGB_GREEN_PIN, !digitalRead(RGB_GREEN_PIN));
      delay(10);
      digitalWrite(RGB_RED_PIN, !digitalRead(RGB_RED_PIN));
      delay(10);
  }

  if (!digitalRead(ENCODER_PIN_BTN)) {  // simulate a reset with a push of the encoder
    //power on all leds shortly
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(YELLOW_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, HIGH);
    for (int i = 0; i < 3; i++) { // animate the leds
      digitalWrite(RGB_RED_PIN, HIGH);
      delay(100);
      digitalWrite(RGB_RED_PIN, LOW);
      digitalWrite(RGB_GREEN_PIN, HIGH);
      delay(100);
      digitalWrite(RGB_GREEN_PIN, LOW);
      digitalWrite(RGB_BLUE_PIN, HIGH);
      delay(100);
      digitalWrite(RGB_BLUE_PIN, LOW);
    }
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);

    display.clear(); //clear the segments and show a 'refresh' animation using the segements
    const uint8_t animationSegments[] = {SEG_A, SEG_B, SEG_G, SEG_E, SEG_D, SEG_C, SEG_G, SEG_F, SEG_A};
    for (int i = 0; i < 9; i++) {
      uint8_t currentSegment = animationSegments[i];
      for (int digit = 0; digit < 4; digit++) {
        display.setSegments(&currentSegment, 1, digit);
      }
      delay(20);
    }

    const uint8_t additionalAnimation[] = {SEG_A, SEG_G, SEG_E, SEG_G};  // fast animation on 7segments just for fun
    for (int j = 0; j < 5; j++) {
      for (int i = 0; i < 4; i++) {
        display.clear();
        display.setSegments(&additionalAnimation[i], 1, i);
        delay(50);
      }
    }
   // show the word "turn" in small caps 
    const uint8_t turnSegments[] = {0x78, 0x3E, 0x50, 0x54}; // t, u, r, n
    for (int i= 0; i < 6; i++){  // blink the led and flash the word 6 times for getting attention
      display.setSegments(turnSegments);
      delay(100);
      display.clear();
      display.setSegments(turnSegments);
      digitalWrite(RGB_GREEN_PIN, HIGH);
      delay(30);
      digitalWrite(RGB_GREEN_PIN, LOW);
      display.clear();
      display.setSegments(turnSegments);
    }

    animationDone = true;  // set var to true so the program konws where it is later on.
  }

  if (animationDone) {  //check the current position of teh encoder
    encoder.tick();
    int encoderPos = encoder.getPosition();

    if (encoderPos != prevEncoderPos) {  // cycle the leds with the encoder
      int pos = abs(encoderPos) % 3;
      digitalWrite(RED_LED_PIN, pos == 0 ? HIGH : LOW);
      digitalWrite(YELLOW_LED_PIN, pos == 1 ? HIGH : LOW);
      digitalWrite(GREEN_LED_PIN, pos == 2 ? HIGH : LOW);
      prevEncoderPos = encoderPos;
      ledCycles++; // update the amount of cycles 
    }

    if (encoder.getDirection() != RotaryEncoder::Direction::NOROTATION) {
      uint8_t randomPattern[4] = { //display random patterns on the 7segment display. just for fun.
        random(256),
        random(256),
        random(256),
        random(256)
      };
      display.setSegments(randomPattern);
    }
  }

  if (!aiCoreBooted) {

    if (ledCycles >= 7 && ledCycles < 12) { //do this only when the leds have cycled 7 times and less than 12 times
      int color = ledCycles % 3;
      digitalWrite(RGB_RED_PIN, color == 0 ? HIGH : LOW); // flash RHB red
      digitalWrite(RGB_GREEN_PIN, color == 1 ? HIGH : LOW); // flash RGB green
      digitalWrite(RGB_BLUE_PIN, color == 2 ? HIGH : LOW); //flash RGB blue
      delay(400 - (ledCycles - 7) * 50);
    }


        if (ledCycles == 12) { // start the A.I. bootsequence and status pages after 12 led cycles with the rotary encoder
          int chargingCounter = 0;
          while (chargingCounter <= 256) {
            int photoresistorValue = analogRead(PHOTORESISTOR_PIN);
            int chargingStep = map(photoresistorValue, 800, 1000, 8, 12);
            // Mapped the photoresistor value for 'booting speed'

            u8g.setRot180();
            u8g.firstPage();
            do {
              u8g.setFont(u8g_font_profont17);
          u8g.setPrintPos(0, 20);
          u8g.print("BOOTING");
          u8g.setPrintPos(0, 40);
          u8g.print("A.I. CORE");
          u8g.setFont(u8g_font_profont12);
          u8g.setPrintPos(0, 55);
          u8g.print("Status: ");
          u8g.print(chargingCounter);
          u8g.print(" / 256");
        } while (u8g.nextPage());

       chargingCounter += chargingStep;
        delay(500); // 500ms pause
      }

        ledIndicator();  // fast eld animation for visual effect
    
        u8g.firstPage();
        do {
          u8g.setFont(u8g_font_profont17);
          u8g.setPrintPos(0, 20);
          u8g.print("A.I. CORE");
          u8g.setFont(u8g_font_profont15);
          u8g.setPrintPos(10, 40);
          u8g.print("OPERATIONAL");
          aiCoreBooted = true;
        } while (u8g.nextPage());
        
        aiCoreBooted = true;
        ledIndicator();  // fast eld animation for visual effect

        
        if (!finalPageReached) {
          if (millis() - lastPageUpdate > pageUpdateInterval) {

            displayAIPage(currentPage);
            ledIndicator();   // fast led animation for visual effect per page
            currentPage++;
            if (currentPage >= totalPages) {
              finalPageReached = true;
            }
            lastPageUpdate = millis();
          }
         }
  
        if (finalPageReached && !animationFinished) {
          drawSpaceMap();
          delay(50);
        }

      }
    }
    else {


  if (!finalPageReached) { // As long as the final page is not reached, show the next page
    if (millis() - lastPageUpdate > pageUpdateInterval) {
      displayAIPage(currentPage);
      ledIndicator();
      currentPage++;
      if (currentPage >= totalPages) { // if the current page is the last page, set var to true
        finalPageReached = true;
      }
      lastPageUpdate = millis();  //wait a little
    }
  } else {
    u8g.firstPage();
    do {
      drawSpaceMap();
    } while (u8g.nextPage());

    if (!animationFinished) {
      // Increment the zoom factor to speed up zoom. that decreases animation performance though.
      zoomFactor += 0.035;

      // Check if the planet is close enough/zommed in enough, and if so, stop the animation
      // 
      if (planet.radius * zoomFactor >= 32) {
        animationFinished = true;
      }
    }
  }
}
}
