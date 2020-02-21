/*
  Project:   mizarTimer
  Source:    http://github.com/pa3hcm/mizarTimer
  Authors:   Ernest Neijenhuis PA3HCM
*/


/////////////////////////////////////////////////////////////////////////
// Constants

// Pins
const byte P_BUZZER  = 13;
const byte P_BUTTON1 = 19;
const byte P_BUTTON2 = 23;
const byte P_SDA     = 21;
const byte P_SCL     = 22;

// Timer length [ms]
const long TIMER = 90 * 60 * 1000;  // 90 minutes * 60 sec/min * 1000/sec
//const long TIMER = 5000; // 5 seconds (for testing purposes)

// Alarm duration [ms]
const long ALARM_DURATION = 10 * 1000;

/////////////////////////////////////////////////////////////////////////
// Global variables

long timeGoal = 0;
//long time_count = 0;
bool toggle = false;


/////////////////////////////////////////////////////////////////////////
// Libraries

// Adafruit SSD1306 display driver source & info:
// https://github.com/adafruit/Adafruit_SSD1306
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 displ(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


/////////////////////////////////////////////////////////////////////////
// interrupt routines

void IRAM_ATTR startTimer() {
  timeGoal = millis() + TIMER;
}

void IRAM_ATTR resetTimer() {
  timeGoal = 0;
}


/////////////////////////////////////////////////////////////////////////
// setup() routine

void setup() {
  Serial.begin(9600);
  while(!Serial);    // time to get serial running

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!displ.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  displ.cp437(true); // Use full 256 char 'Code Page 437' font

  pinMode(P_BUZZER, OUTPUT);
  pinMode(P_BUTTON1, INPUT);
  pinMode(P_BUTTON2, INPUT);

  attachInterrupt(P_BUTTON1, resetTimer, FALLING);
  attachInterrupt(P_BUTTON2, startTimer, FALLING);
}


/////////////////////////////////////////////////////////////////////////
// Main loop()

void loop() { 
  displ.clearDisplay();
  
  // Draw frames
  displ.fillRect( 0,  0, 128, 64, SSD1306_WHITE);
  displ.fillRect( 1, 12, 126, 51, SSD1306_BLACK);

  // Labels
  displ.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  displ.setTextSize(1);
  displ.setCursor(4, 2);
  displ.print("Mizar Timer");

  // Contents
  displ.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  displ.setTextSize(3);
  displ.setCursor(20, 26);
  int countdownMin = (timeGoal - millis()) / 60000;
  int countdownSec = ((timeGoal - millis()) / 1000) - (countdownMin * 60);
  char countdownMinStr[3];
  char countdownSecStr[3];
  //dtostrf(countdownMin, 2, 0, countdownMinStr);
  sprintf(countdownMinStr, "%02d", countdownMin);
  sprintf(countdownSecStr, "%02d", countdownSec);
  if (toggle) {
    toggle = false;
  } else {
    toggle = true;
  }
  if (millis() < timeGoal) {  // We are counting down...
     displ.print(countdownMinStr);
    if (toggle) {
      displ.print(":");
    } else {
      displ.print(" ");
    }
    displ.print(countdownSecStr);
  } else if(millis() >= timeGoal and millis() < (timeGoal + ALARM_DURATION)) {  // Alarm!
    if (toggle) {
      displ.print("Mizar");
      beep();
      // tone(P_BUZZER, 880, 50); --> tone() is not supported on ESP32 :(
      
    } else {
      displ.print("     ");
      beep();
    }
  } else {  // Nothing to do...
    displ.print("00:00");
  }

  displ.display();
  delay(470);  // 500ms - 30ms (a full loop takes about 30ms)
}

void beep() {
  for (int i=1; i<50; i++) {
    digitalWrite(P_BUZZER, HIGH);
    delay(1);
    digitalWrite(P_BUZZER, LOW);
    delay(1);
  }
}
